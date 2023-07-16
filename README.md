# Toy TSTAT demo

This project demonstrates a flexible architecture to implement a toy thermostat
comprising two modules, a tstat-logic" module and a tstat-model module.  A
broker module controls the flow of messages between the two modules.

The modules may be implmented in hardware or emulated in software.  Berkeley
sockets provide the transport channels among the modules and broker.  Messages
themselves are null-terminated JSON strings.

```
           +----------+     +----------+     +----------+
           |  TSTAT   |     |          |     |  TSTAT   |
hardware   |  LOGIC   |<===>|          |<===>|  MODEL   |
           |          |     |          |     |          |
           +----------+     |          |     +----------+
                            |  BROKER  |
           +----------+     |          |     +----------+
           |  TSTAT   |     |          |     |  TSTAT   |
emulated   |  LOGIC   |<===>|          |<===>|  MODEL   |
           |          |     |          |     |          |
           +----------+     +----------+     +----------+
```

## `tstat-logic`
`tstat-logic` controls the behavior of the thermostat.  It periodically
requests a copy of the `tstat-model`'s state in order to get the system mode,
setpoints and ambient temperature.  Based on that information, it computes new
values for the relays and sends the updated state back to `tstat-model`.

`tstat-logic` may generate the following messages:

## `tstat-model`
`tstat-model` is responsible for maintaining the thermostat's state information,
including system mode, setpoints, ambient termperature and relay state.  It is
entirely passive, in that it does not update any of its own state.

`tstat-model` will respond the the following messages:
```
{"topic":"tstat-model", "fn":"get_state", "args":{}} =>
  {"topic":"tstat-logic",
   "fn":"report_state":,
   "args":{"ambient":<temperature_t>,
           "cool_setpoint":<temperature_t>,
           "heat_setpoint":<temperature_t>,
           "relay_y":<bool>,
           "relay_w":<bool>,
           "system_mode":<int>}}
{"topic":"tstat-model", "fn":"set_state", "args":{}}

```
