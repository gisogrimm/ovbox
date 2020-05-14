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

## Components

### users:

* username
* device names

### peers:

* user
* address (IP/port) - only while active
* position

### rooms:

* list of peers
* address (IP/port) of room service
* lock (and other properties)

### house:

* list of rooms
* list of users

## Registration workflow

1. A registered user enters the lobby
2. The list of available rooms is displayed
3. The user selects a room
4. The device connects through the back door of the lobby
5. The room key and location as well as the chair numberwithin the
   room is provided to the device
6. The device connects to the room service using the room key and chair number
7. The audio of the device is transmitted to all other devices, and
   the audio of all other devices is transmitted to this device

The room is cleaned as soon as all members left the room. A new key is
generated.

Within a room the members are arranged either freely or in a
circle. When arranged in a circle, a user can swap positions with
other users. These changes will affect the listening position at all
devices instantly. A room can be locked from inside. A locked room can
not be entered.

Each room service reports the availability of a room to the lobby.

## Structure and protocols

The lobby is a server with three interfaces:

* User interface: http
* Device interface: http or websocket
* Room service interface: http or websocket

A room service is client and server with two interfaces:

* Lobby interface (client): http or websocket
* Device interface (server): dedicated protocol, UDP transport, RTP audio

A device is a client and server with two interfaces:

* Room service interface (client): dedicated protocol
* Mixer/control interface (server): http

## Prototype implemtation

* User authentication: web server/htaccess
* Device database: text files, name = device name, created / modified upon user input
* Room database: text files, name = room id, created by room service connection

Example room file (host:port pin-code):
````
mplx.yourdomain.com:4464 1234
````

Example device file (roomname pos-x pos-y pos-z)

````
room1 2 3 0
````

The device first gets the device file. Then it checks out the room
file, and connects to the room service. The own position is sent to
the room service upon registration. The names and positions of the
other participants is received from the room service. The device sets
up the renderer and receives/sends audio.

In periodic follow-up requests (e.g., every 5 seconds) only the device
file is requested. If the room has changed or is empty, then a new
session is established. If the position changed, then the renderer is
updated and the new position is transmitted to the room service.