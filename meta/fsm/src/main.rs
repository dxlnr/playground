// OBJECTS
#[derive(Debug)]
struct ActiveUser {
    name: String,
    msg_count: u32,
}
#[derive(Debug)]
#[allow(dead_code)]
struct BannedUser {
    name: String,
    reason: String,
}
// EVENTS
struct SendMessage;
struct Ban {
    reason: &'static str,
}
// TRANSITIONS
trait Transition<Event> {
    type NextState;

    fn next(self, event: Event) -> Self::NextState;
}
// ActiveUser -- SendMessage -- ActiveUser
impl Transition<SendMessage> for ActiveUser {
    type NextState = ActiveUser;

    fn next(mut self, _e: SendMessage) -> Self::NextState {
        self.msg_count += 1;
        self
    }
}
// ActiveUser -- Ban -- BannedUser
impl Transition<Ban> for ActiveUser {
    type NextState = BannedUser;

    fn next(self, e: Ban) -> Self::NextState {
        BannedUser {
            name: self.name,
            reason: e.reason.to_string(),
        }
    }
}
// Generic step() function
fn step<S, E>(state: S, event: E) -> S::NextState
where
    S: Transition<E>,
{
    state.next(event)
}

fn main() {
    let u0 = ActiveUser { name: "illnerd".to_string(), msg_count: 0 };
    // Send messages
    let a1 = step(u0, SendMessage);
    println!("a0: {:?}", a1);
    let a2 = step(a1, SendMessage);
    println!("a1: {:?}", a2);
    let a3 = step(a2, SendMessage);
    println!("a2: {:?}", a3);
    // Ban the user
    let a4 = step(a3, Ban { reason: "just so" });
    println!("a3: {:?}", a4);
    // // INVALID TRANSITION
    // let oops = step(a4, SendMessage);
}
