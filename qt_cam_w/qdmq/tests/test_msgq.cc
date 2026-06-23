#include "test_helper.h"

#include "qdmq/msgq.h"

void test_msgq_msg_init_size(void)
{
  size_t msg_size = 32;
  msgq_msg_t msg;
  msgq_msg_init_size(&msg, msg_size);
  TEST_ASSERT(msg.size == msg_size);
  msgq_msg_close(&msg);
}

void test_msgq_msg_init_data(void) 
{
  const size_t msg_size = 32;
  char *data = new char[msg_size];

  for (size_t i = 0; i < msg_size; i++)
  {
    data[i] = i;
  }
  msgq_msg_t msg;
  msgq_msg_init_data(&msg, data, msg_size);

  TEST_ASSERT(msg.size == msg_size);
  TEST_ASSERT(memcmp(msg.data, data, msg_size) == 0);

  delete[] data;
  msgq_msg_close(&msg);

#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/qdm/test_queue");
#endif
}

void test_msgq_init_subscriber(void)
{
  msgq_queue_t q;
  msgq_new_queue(&q, "test_queue", 1024);
  TEST_ASSERT(*q.num_readers == 0);

  q.reader_id = 1;
  *q.read_valids[0] = false;
  *q.read_pointers[0] = ((uint64_t)1 << 32);
  *q.write_pointer = 255;

  msgq_init_subscriber(&q);
  TEST_ASSERT(q.read_conflate == false);
  TEST_ASSERT(*q.read_valids[0] == true);
  TEST_ASSERT((*q.read_pointers[0] >> 32) == 0);
  TEST_ASSERT((*q.read_pointers[0] & 0xFFFFFFFF) == 255);

#ifdef _WIN32
  msgq_close_queue(&q);
  DeleteFileA("test_queue");
#elif __APPLE__
  // TODO
#else
  remove("/dev/shm/qdm/test_queue");
#endif
}

void test_msgq_msg_send(void)
{
  msgq_queue_t q;
  msgq_new_queue(&q, "test_queue", 1024);
  msgq_init_publisher(&q);

  TEST_ASSERT(*q.write_pointer == 0);

  size_t msg_size = 128;

  char *data = new char[msg_size];

  for (size_t i = 0; i < msg_size; i++)
    data[i] = i;

  msgq_msg_t msg;
  msgq_msg_init_data(&msg, data, msg_size);

  msgq_msg_send(&msg, &q);
  TEST_ASSERT(*(int64_t *)q.data == msg_size);
  TEST_ASSERT(*q.write_pointer == 128 + sizeof(int64_t));
  TEST_ASSERT(memcmp(q.data + sizeof(int64_t), data, msg_size) == 0);

  delete[] data;
  msgq_msg_close(&msg);

#ifdef _WIN32
  msgq_close_queue(&q);
  DeleteFileA("test_queue");
#elif __APPLE__
  // TODO
#else
  remove("/dev/shm/qdm/test_queue");
#endif
}

void test_msgq_msg_send_wraparound(void)
{
  msgq_queue_t q;
  msgq_new_queue(&q, "test_queue", 1024);
  msgq_init_publisher(&q);

  TEST_ASSERT((*q.write_pointer & 0xFFFFFFFF) == 0);
  TEST_ASSERT((*q.write_pointer >> 32) == 0);

  const size_t msg_size = 120;
  msgq_msg_t msg;
  msgq_msg_init_size(&msg, msg_size);

  for (int i = 0; i < 8; i++)
  {
    msgq_msg_send(&msg, &q);
  }
  // Check 8th message was written at the beginning
  TEST_ASSERT((*q.write_pointer & 0xFFFFFFFF) == msg_size + sizeof(int64_t));
  // Check cycle count
  TEST_ASSERT((*q.write_pointer >> 32) == 1);
  // Check wraparound tag
  char *tag_location = q.data;
  tag_location += 7 * (msg_size + sizeof(int64_t));
  TEST_ASSERT(*(int64_t *)tag_location == -1);

  msgq_msg_close(&msg);

#ifdef _WIN32
  msgq_close_queue(&q);
  DeleteFileA("test_queue");
#elif __APPLE__
  // TODO
#else
  remove("/dev/shm/qdm/test_queue");
#endif
}

void test_msgq_write_read_msg(void)
{
  const size_t msg_size = 128;
  msgq_queue_t writer, reader;

  msgq_new_queue(&writer, "test_queue", 1024);
  msgq_new_queue(&reader, "test_queue", 1024);

  msgq_init_publisher(&writer);
  msgq_init_subscriber(&reader);
  // Build 128 byte message
  msgq_msg_t outgoing_msg;
  msgq_msg_init_size(&outgoing_msg, msg_size);

  for (size_t i = 0; i < msg_size; i++)
  {
    outgoing_msg.data[i] = i;
  }
  TEST_ASSERT(msgq_msg_send(&outgoing_msg, &writer) == msg_size);

  msgq_msg_t incoming_msg1;
  TEST_ASSERT(msgq_msg_recv(&incoming_msg1, &reader) == msg_size);
  TEST_ASSERT(memcmp(incoming_msg1.data, outgoing_msg.data, msg_size) == 0);

  // Verify that there are no more messages
  msgq_msg_t incoming_msg2;
  TEST_ASSERT(msgq_msg_recv(&incoming_msg2, &reader) == 0);

  msgq_msg_close(&outgoing_msg);
  msgq_msg_close(&incoming_msg1);
  msgq_msg_close(&incoming_msg2);

#ifdef _WIN32
  msgq_close_queue(&writer);
  msgq_close_queue(&reader);
  DeleteFileA("test_queue");
#elif __APPLE__
  // TODO
#else
  remove("/dev/shm/qdm/test_queue");
#endif
}

void test_msgq_pub_slow_sub(void)
{
  msgq_queue_t writer, reader;

  msgq_new_queue(&writer, "test_queue", 1024);
  msgq_new_queue(&reader, "test_queue", 1024);

  msgq_init_publisher(&writer);
  msgq_init_subscriber(&reader);

  int n_received = 0;
  int n_skipped = 0;

  for (uint64_t i = 0; i < 1e5; i++)
  {
    msgq_msg_t outgoing_msg;
    msgq_msg_init_data(&outgoing_msg, (char *)&i, sizeof(uint64_t));
    msgq_msg_send(&outgoing_msg, &writer);
    msgq_msg_close(&outgoing_msg);

    if (i % 10 == 0)
    {
      msgq_msg_t msg1;
      msgq_msg_recv(&msg1, &reader);

      if (msg1.size == 0)
      {
        n_skipped++;
      }
      else
      {
        n_received++;
      }
      msgq_msg_close(&msg1);
    }
  }
  TEST_ASSERT(n_received == 8572);
  TEST_ASSERT(n_skipped  == 1428);

#ifdef _WIN32
  msgq_close_queue(&writer);
  msgq_close_queue(&reader);
  DeleteFileA("test_queue");
#elif __APPLE__
  // TODO
#else
  remove("/dev/shm/qdm/test_queue");
#endif
}
