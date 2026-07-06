# swiftunnel

A bidirectional FEC-protected UDP tunnel built on top of the SWiF codec (Sliding Window FEC with Random Linear Coding over GF(2^8)).

## Files

| File | Description |
|------|-------------|
| `swiftunnel.c` | Tunnel application source |
| `Makefile` | Build rules |
| `swiftunnel_netns.sh` | Network namespace test script |

## Building

```
make -C ../../src/
make
```

## Test script — `swiftunnel_netns.sh`

`swiftunnel_netns.sh` sets up a two-host topology entirely within a single machine using a Linux network namespace, runs the tunnel under configurable packet loss and delay, and reports FEC recovery statistics.

```
 ┌──────────────────────┐          ┌──────────────────────┐
 │  root namespace      │          │  namespace ns1       │
 │                      │          │                      │
 │  tun0 192.168.10.1   │          │  tun0 192.168.10.2   │
 │  swiftunnel (client) │          │  swiftunnel (server) │
 │                      │          │                      │
 │  veth1 10.1.1.1 ─────┼──netem───┼─ vpeer1 10.1.1.2     │
 └──────────────────────┘          └──────────────────────┘
```

The veth pair carries the FEC-encoded UDP traffic and is shaped with `tc netem` to inject delay and loss.  
The TUN interfaces carry plain IP traffic (ping) between the two tunnel endpoints.

### Key parameters (top of the script)

| Variable | Default | Meaning |
|----------|---------|---------|
| `K` | 7 | Source symbols per repair symbol |
| `W` | 50 | Coding window size |
| `DELAY` | 20 | One-way netem delay (ms) |
| `LOSS` | 2 | Packet loss percentage |

### Usage

```bash
cd applis/swiftunnel
make
sudo bash swiftunnel_netns.sh
```
