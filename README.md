# superslew
Slew/ground control plugin for X-Plane 10

- Move your plane around. Joystick moves forwards/backwards or sideways, throttle
controls speed. The yaw control rotates the plane.

- The slow speed setting can be used to park or push back your plane. The faster
settings lets you explore the scenery around you as well.

- The plane is kept on the ground, even across hills, but be careful
  if you end up partly in the ground; you will "crash" when you disable
  the SuperSlew mode.

- If you enable altitude mode, joystick right/left moves plane
up/down. You can not go below your starting altitude.

- There are three speeds to choose from. The first one has a maximum
  speed of 8 m/s. The second setting has a maximum speed of 128 m/s,
  and the third one has a maximum speed of 512 m/s. The latter
  corresponds to almost 1000 knots. Maximum speed is reached with
  maximum trottle.

- If you move really far and fast you may get ahead of the scenery loading,
but it usually fixes itself when you wait a bit.

- Shows a dialog box in the lower right corner when enabled, showing
the joystick input values, altitude, and geographical position.

- Defines a custom command called `durian/superslew/toggle` which can be
  bound to a key or joystick button.

- You may need to move your controls a bit before enabling SuperSlew.

- It may not parse your joysticks properly if you have had different
  controls connected at different times. X-Plane seems to leave traces
  in the configuration files confusing SuperSlew. The only cure is to
  throw away your preferences and configure them again.

- Only tested with a simple Saitek joystick and CH Products pedals.

- Can be enabled "in flight" which sometimes causes spectacular effects
  when disabled again.

- The orientation of the plane is fixed, which may cause problems when
  moving very large distances.
