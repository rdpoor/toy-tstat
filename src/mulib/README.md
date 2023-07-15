# Welcome to `mulib`


`mulib` a highly performant, compact and reliable framework for building complex
applications in resource-constrained embedded systems.  

`mulib`'s growing list of modules include:
* `mu_sched`: A fast and lithe run-to-completion (single threaded) scheduler,
which also supports safe, efficient linkage from interrupt to foreground levels.
* `mu_task`: Support for deferred execution
* `mu_strbuf` and `mu_str`: Fast and safe copy-free string handling.
* `mu_access_mgr`: Grant exclusive access to a shared resource.

## `mulib`'s philosophy:  Small, Fast and Trusting

`mulib`'s design is guided by the following tenets:

* **Self-contained, pure C code**. `mulib` minimizes dependencies on external
libraries, except for the most common ones such as `stdint` and `stdbool`.
* **No malloc.  Ever.** `mulib`'s modules accept user-allocated data structures.
If your application already has malloc, you're free to use it, but `mulib` will
never malloc behind your back.
* **Well tested and supported.** `mulib` is validated through ample unit testing
and continuous integration.
* **Super portable.** To port `mulib` to a new target environment, you only
need create `mu_platform.h` and `mu_platform.c`, and those files may already exist
in the mu_examples repository.
* **Fast and Trusting.** `mulib` favors maximizing speed and minimizing code
space over safety.  In general, `mulib` trusts that you're passing valid
parameters and that your code implements argument validation where needed.
* **Single-threaded design.** Much of `mulib`'s small code footprint and
low-overhead is attributed to its single thread / multiple task design.
See [Run To Completion Schedulers](./docs/RunToCompletion.md)
for the reasons -- and implications -- behind this choice.
* **Power-aware scheduling.** `mulib`'s architecture for handling interrupts 
and scheduling make it easy to create low-power embedded applications. 
* **Low threshold, high ceiling.** A suite of well-documented code examples
guide you from the simplest "blink an LED" demo to complex multi-tasking
applications.
* **Yours to use.** The entirety of `mulib` is covered under the permissive MIT
Open Source license.

(And in case you're wondering about the pronunciation, "MOO-lib" is preferred
over "MEW-lib": cows, not kittens.)

## Getting Started

Click on one of the links, depending on what you're looking for:

* [**Browse the `mulib` tutorial code.**](#browse_tutorials)
* [**Run a `mulib` example application.**](#running_examples)
* [**Create a `mulib`-based application.**](#create_a_project)
* [**Port `mulib` to a new processor or platform.**](#porting_mulib)

## Browse the `mulib` code examples.<a name="browse_tutorials"></a>

Work In Progress: link to https://github.com/rdpoor/mulib-examples/tree/main/tutorials

## Run a `mulib` example application.<a name="running_examples"></a>

Work In Progress: link to https://github.com/rdpoor/mulib-examples

## Create a `mulib`-based application.<a name="create_a_project"></a>

Work In Progress: link to https://github.com/rdpoor/mulib-examples/platforms,
which will be a repository for /platform directories for various platforms.

## Port `mulib` to a new processor or platform.<a name="porting_mulib"></a>

Work In Progress: link to https://github.com/rdpoor/mulib-examples/porting,
suggest copying an existing platform and modifying it and/or using the template
files.
