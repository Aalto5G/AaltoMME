MME Architecture
================

Folder structure
----------------

The folder structure is the following:

```
$ tree -d
.
|-- Common
|-- NAS
|   |-- include
|   |-- shared
|   |-- src
|   `-- testApp
|-- S1AP
|   |-- include
|   |-- shared
|   |-- specs
|   |-- src
|   `-- testApp
|       `-- binaries
|-- TestProgram
|-- cmake
|-- doc
|-- exampleProgram
|   `-- binaries
|-- libgtp
|   `-- include
|-- m4
|-- mme
|   |-- Controller
|   |-- S1
|   |   `-- NAS
|   |-- S11
|   `-- S6a
|       |-- hmac
|       |-- milenage
|       `-- scripts
`-- test

```

As a convention, the headers are separated on include and shared folders.
The headers on include folders are private headers of the module.
The headers on shared folders are public headers to be included when the module is used

- `Common`: Common libraries used on multiple parts of the source tree

- `NAS`: Non-Access Stratum (NAS) messages.
This library provides functions to encode and decode NAS messages

- `S1AP`: S1AP protocol messages.
This library provides functions to encode and decode S1AP messages

- `TestProgram`: Very old tests cases.
They used the check test framework.
Not used anymore

- `cmake`: files required to compile using cmake

- `doc`: Documentation folder

- `exampleProgram`: Very old small programs used for development. Not used anymore

- `libgtp`: GTPv2 library to encode and decode GTPv2 messages.
It was based on an older library, implementing older versions of the protocol.

- `m4`: folder to place m4 scripts for autotools build system.
Not used anymore.

- `mme`: MME main source code

- `test`: Glib based unit tests.
Basically testing NAS encryption and integrity algorithms.

### Structure of mme folder

The folders inside the `mme` folder implement different MME interfaces.

- `Controller`: This folder implements the interface towards a SDN controller to push certain events.
It was for research purposes and it is not used normally

- `S1`: This folder contains the State machine implementations of the S1 interface.

- `S1/NAS`: This folder contains the state machine implementations of the NAS sublayers: EMM and ESM.

- `S11`: This folder contains the state machine of the S11 interface.
A S10 interface could reuse the same GTPv2 endpoint, so it is recommended to implement future S10 state machines on this folder and rename it to S11S10

- `S6a`: This interface contains the integration between the MME and the HSS, the mock S6a interface.
This will be modified when a real S6a interface is implemented.

- `MME.c/MME.h`: Files containing the main MME structure and related functions

- `MME_test.c`: Main function for the MME binary

- `MMEutils.c/MMEutils.h`: Common functions

- `Subscription.c/Subscriptions.h`: Subscription structures and functions used to interface S6a with the rest of the code

- `commands.c/commands.h`: Very simple management interface to the MME

- `nodemgr.c/nodemgr.h`: Configuration manager code used to read mme.cfg.
In the future, the nodes should be detected by DNS

Design
------

### Asynchronous

The MME uses an asynchronous design based on [libevent][libevent].
The design is inspired by the [C10K][c10k] problem.

In practice this means that the main application structure is quite simple:

1. A setup phase when sockets are registered to the ports of interest
2. A serving phase when the application block on a main loop waiting incoming packets.

When a packet is received, the registered callback will process that packet and return to the main loop.
It is very important that blocking calls are avoided, otherwise, the application will stop serving requests.


### Object Oriented C

The design follows the Object oriented C paradigm.
The first argument of a function corresponds to the object being modified or accessed.
The internals of the struct are not shown on the external interface.
This is done by dereferencing the pointer to a (void*).
If some information from the struct is required by an external module, a new accessor method should be implemented.
Never cast the pointer to the internal struct from outside the module.

### Other code conventions

The code tries to follow the advises exposed [here][howtoc]

[howtoc]: https://matt.sh/howto-c
[c10k]: http://www.kegel.com/c10k.html
[libevent]: http://libevent.org
