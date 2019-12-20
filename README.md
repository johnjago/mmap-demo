# mmap-demo

A program to demonstrate memory-mapped files using the mmap(2) system call in Unix.

## Build

```
make
```

## Usage

To take resources (from resources.txt):

```
./demo.out take
```

To provide resources (to resources.txt):

```
./demo.out prov
```

Follow the prompts to either take or provide resources. The file will be updated
in memory and then synced to the disk. When asked for the resource number and
number of resources, separate the two with a space.

### Example

resources.txt before:

```
0	0
1	9 <-----
2	6
3	3
4	0
```

```
$ ./alloc.out take
Take more resources? (y/n) y
Enter the resource number and number of resources needed: 1 1
Synced successfully.
```

resources.txt after:

```
0	0
1	8 <-----
2	6
3	3
4	0
```

## License

Unlicense
