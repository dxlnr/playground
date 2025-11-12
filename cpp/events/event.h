#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <csignal>
#include <unistd.h>

class EventSignal {
public:
    using Handler = std::function<void(int)>;

    /**
     * Construct and register a SIGUSR1 handler.
     * @param handler Called when a SIGUSR1 is caught (sender PID passed).
     */
    explicit EventSignal(Handler handler = nullptr);

    /// Change the callback handler at runtime.
    void setHandler(Handler handler);

    /**
     * Send a notification (SIGUSR1) to another process.
     * @param pid Process ID to notify.
     */
    void notify(pid_t pid) const;

    /// Internal static handler registered with sigaction.
    static void signalHandler(int signo, siginfo_t* info, void* context);

private:
    Handler handler_;
    static EventSignal* instance_;
};

#endif // EVENT_H
