 Minimal code to crash the ENet ([Issue #134](https://github.com/lsalzman/enet/issues/134)).


 The code is nothing fancy.
 Glued parts from the tutorial together to establish connection on the localhost and start sending
 packets with 2K data and flags ` ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT`


compile:

 ```
 gcc ./server.c -lenet -o server
 gcc ./client.c -lenet -o client
 ```
 establish "suitable" network conditions with packet reordering:

```
  sudo tc qdisc add dev lo root netem gap 2 delay 10ms reorder 100
                         ^-localhost      ^- some packet reordering (These values "work" for the demonstration)
```
 start the server:

  ```
  ./server &
 [1] 30154
 ```
start the client

  ```
  ./client

  Connection to 127.0.0.1:1234 succeeded.
  Expecting to crash soon.
  A new client connected from 100007f:48210.
  A packet of length 2000 was received on channel 0.
  Segmentation fault (core dumped)
```
 to clear the packet reordering (after the test):

  ```
  sudo tc qdisc del dev lo root
  ```
