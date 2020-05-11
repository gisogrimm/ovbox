# Our vision

This document describes the vision we have in mind: a virtual 'house
of consort', with rooms where ensembles can meet.

Users of the ensemble service can register their client(s) on a
central server. With each device it is possible to enter virtual
rooms. A room can be locked from inside, i.e., any member inside a
room can lock it so others can not enter it. At any time it is
possible to leave a room. It is possible to see from outside who is in
a room. In a first step, room members sit in a circle. The position
with other room members can be swapped.

A client can be either a hardware box, or a software client, e.g.,
app, web interface etc.

Each room is managed by a room service. This is a server listening at
a port. The room information is taken from the central ensemble
server: list of participants, order of participants. The clients query
the room service location from the central server. The room service
shares the list of room members and their positions with the client
regularly.

Audience clients are special clients for listening only. These could
be broadcasting tools, or interactive clients for end listeners, e.g.,
for an interactive concert experience. Room members can control wether
audience clients are allowed in a room or not.

Public rooms are reset to its default state when the last room member
leaves the room. Personal rooms can be managed by individuals.
Entering personal rooms is possible only upon invitation.

The room service collects connection statistics and transfers it to
the central server upon request.

On the central server, clients can be configured to connect to other
public services, e.g., the digital stage project.


## Relation to current structure

In the current ad-hoc solution, only a single room exists, with
manually configured members and locations. The room service
corresponds to the `mplx_server`, and the client communication is
handled by `mplx_client`.

The current hand-woven solution with multiple zita-njbridge instances
will be replaced by an RTP client.

Virtual acoustic sessions will be created and modified on the fly upon
configuration changes reported from the room service.
