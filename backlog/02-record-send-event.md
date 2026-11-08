---
task: 02
title:  Implement Event functionality
status: done
---
Now that the timer functionality work, we will focus on the event part.

# Recording Event to be send

To configure the event to be send
a record button will be add next to the Event field.

When pressed the record button, will listen to keyboard event,
and once a keyboard event is received it will be store as the event to send when the timer reach 0.

Ideally some text info describing the event has to be displayed in the event field.
We don't focus to much on this part, as long it gives enough info to distinguish one event from an other one.

Ideally it should display modifier key (Alt / Ctl / shift) + the event key used.

Event recorded should survive application shutdown and be stored in a user configuration.

# Sending the Event when reaching 0

Once an Event has been recorded it is send every time the timer reach 0.
We will keep the modal windows to signal that this has happened.

# Target application (follow-up)

Some applications (for example OBS) only process their global hotkeys from real
XInput2 key events and ignore XTEST events injected by `xdotool` while the
application is unfocused. To support them, a target application can be picked
with the `Pick` button and stored in the user configuration. When a target is
set, the event is sent directly to that window with
`xdotool key --window <id> --clearmodifiers <sequence>`.
