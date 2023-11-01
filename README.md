# You Spin Me Round Robin

A process scheduler that uses First Come First Serve + Preemption, also known as the Round Robin Algorithm. Parses input file of processes' pid, arrival time, and burst time. Then calculates the average wait and response time based on a specified quantum as the second argument (first being the txt that contains the arrival and burst times). Also, if the quanta specified is the string "median", it changes the algorithm to recalculate the current quantum length to be the median of all CPU time exhausted by all programs arrived.  

## Building

```shell
make
```

## Running

cmd for running TODO
```shell
./rr process.txt quantum_number <--- (or string "median)
```

results TODO
```shell
example:
process.txt file contains:
4
1, 10, 70
2, 20, 40
3, 40, 10
4, 50, 40

make
./rr process.txt 10

yields the results:
Average wait time: 70.00
Average response time: 12.25

```

## Cleaning up

```shell
make clean
```
