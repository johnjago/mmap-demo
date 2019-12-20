# mmap-demo

Programs to demonstrate memory-mapped files in Unix.

## Build

```
make
```

## Usage

Run alloc.out and follow the prompts to allocate resources (subtract from)
resources.txt. Run provide.out in another terminal to provide more resources to
resources.txt. provide.out also prints out status information every ten seconds in a
child process.

Note that you'll have to kill the child process by hand to stop provide.out from
printing.

## License

Unlicense
