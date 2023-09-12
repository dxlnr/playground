def sdbm(input):
    h = 0
    for i in range(len(input)):
        c = ord(input[i]) << ((i % 4) * 8)
        h = c + (h << 6) + (h << 16) - h
    return h


print("Hello  : ", (sdbm("Hello")))
print("Helo   : ", (sdbm("Helo")))
print("Hello3 : ", (sdbm("Hello3")))
print("Hllo   : ", (sdbm("Hllo")))
print("Hello  : ", (sdbm("Hello")))
