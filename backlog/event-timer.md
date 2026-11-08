---
task: 01
title:  Implement basic functionality
status: todo
---

Create a simple count down timer app with native UI.

The app allows user:
- to define a time limits (hours minute second)
- to define an event key to be send to the system when the timer reach 0.
- to start, pause en reset the timer to it's initial value
- every second the remaining time will be updated.

Visual should be very simple and straightforward.

To implement the app:
- we will use C++ and meson
- For the GUI we will use https://github.com/slint-ui/slint?utm_source=chatgpt.com#why-slint
(this is also a experiment for slint)

In that story we don't focus on how event will be sent when timer expire, only on the basic UI & user experience (setup & trigger).

When timer coutdown expire, a simple modal window will be displayed here.