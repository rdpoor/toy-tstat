# Toy TSTAT demo

This project demonstrates a flexible architecture to implement a toy thermostat
comprising two modules, tstat-logic and tstat-model.  A broker controls the flow
of messages between the two modules.

The modules may be implmented in hardware or emulated in software.  Berkeley
sockets provide the transport channels among the modules and broker.

```
           +----------+----+   +----------+   +----+----------+
           |  TSTAT   |ser/|   |          |   |ser/|  TSTAT   |
hardware   |  LOGIC   |sock|<->|          |<->|sock|  MODEL   |
           |          |    |   |          |   |    |          |
           +----------+----+   |          |   +----+----------+
                               |  BROKER  |
           +----------+        |          |        +----------+
           |  TSTAT   |        |          |        |  TSTAT   |
emulated   |  LOGIC   |<------>|          |<------>|  MODEL   |
           |          |        |          |        |          |
           +----------+        +----------+        +----------+
                                    ^
                                    |
                                    v
                               +----------+
                               |   TEST   |
                               | HARNESS  |
                               +----------+
```

## `tstat-logic`
`tstat-logic` controls the behavior of the thermostat.  It periodically
requests a copy of the `tstat-model`'s state in order to get the system mode,
setpoints and ambient temperature.  Based on that information, it computes new
values for the relays and sends the updated state back to `tstat-model`.

The hardware implementation is written in C and is designed to run on resource
constrained microcontrollers.

The emulated implementation may be written in C -- using the same code base as
the hardware implementation -- or in Python.

## `tstat-model`
`tstat-model` is responsible for maintaining the thermostat's state information,
including system mode, setpoints, ambient termperature and relay state.  It is
"passive", meaning that it never modifies its own state.

The toy implementation maintains the following state:

```
typedef struct {
    temperature_t ambient;
    temperature_t cool_setpoint;
    temperature_t heat_setpoint;
    bool relay_y;
    bool relay_w;
    int system_mode;
} tstat_model_t;
```

The hardware implementation is written in C and is designed to run on resource
constrained microcontrollers.

The emulated implementation may be written in C -- using the same code base as
the hardware implementation -- or in Python.

## `broker`
The `broker` is responsible for routing messages among the modules.  It uses the
Berkeley `socket` protocol to `send()` and `recv()` messages.  The reference
implementation is written in Python.

## `ser/sock` bridge
The `ser/sock` bridge is a small bit of code that translates between serial data
and socket data.  The reference implementation is written in Python.

## Open issues

What is the format of a message?  Does it include encapsulation with routing
information on the outer layers and application specific data inside?

What is the framing of a message?  Assuming everything is JSON/ASCII, do we
terminate with a newline?  Or with a null?  I think a null termination is
preferable, since it's an out-of-band value for JSON and there's no ambiguity
if its '\r\n' or '\n'.
