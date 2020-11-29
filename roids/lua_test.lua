print("Lua is loading!")

radius = 150;   -- how large the collision circle and shield are

mass = 100;   -- how heavy in collisions

-- each pre-scaled for DT of 1/1000
turnRate = .075;       -- how fast it turns .... 0.075 = 75 deg/S
enginePower = .000125;  -- how fast it accelerates .... 0.00125 = 1250 PPS^2
maxSpeed = 1.0;        -- some kind of max speed ... 1.0 = 1000 PPS

shieldPoints = 100;

weapons = {
  {
    x = 2,         -- position on the model
    y = 0,
    angle = 0,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 50,   -- compared to 1000 updates per second
    glowLimit = 30,
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_DOUBLE',
    shotmod = 'SHOT_FAST_LONG'
  }
}

engines = {
  {
    firingMod = 6,
    firingPoint = 2,
    x = -102,   -- position on the model
    y = 24
  },
  {
    firingMod = 6,
    firingPoint = 4,
    x = -102,
    y = -24
  },
  {
    firingMod = 6,
    firingPoint = 0,
    x = -102,
    y = 0
  }
}

function draw()

    --front head SKUG
    glColor3f(1,1,1)
    glBeginPolygon()
      glVertex2i(96,-30)
      glVertex2i(96,30)
      glVertex2i(120,18)
      glVertex2i(120,-18)
    glEnd()

--middle head SKUG
glColor3f(1,0,0)
glBeginPolygon()
  glVertex2i(96,-30)
  glVertex2i(96,30)
  glVertex2i(72,30)
  glVertex2i(72,-30)
glEnd()

    --lower head SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(48,-18)
    glVertex2i(48,18)
    glVertex2i(72,30)
    glVertex2i(72,-30)
  glEnd()

    -- red neck SKUG
  glColor3f(1,0,0)
  glBeginPolygon()
    glVertex2i(48,-18)
    glVertex2i(48,18)
    glVertex2i(0,18)
    glVertex2i(0,-18)
  glEnd()
  -- white body front SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-30,-36)
    glVertex2i(-30,36)
    glVertex2i(0,18)
    glVertex2i(0,-18)
  glEnd()

  --big bad red box SKUG
  glColor3f(1,0,0)
  glBeginPolygon()
    glVertex2i(-30,-36)
    glVertex2i(-30,36)
    glVertex2i(-96,36)
    glVertex2i(-96,-36)
  glEnd()

  --left wing SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-30,36)
    glVertex2i(-124,14*6)
    glVertex2i(-124,60)
    glVertex2i(-96,36)
  glEnd()

  --right wing SKUG
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-30,-36)
    glVertex2i(-124,-14*6)
    glVertex2i(-124,-60)
    glVertex2i(-96,-36)
  glEnd()

   --motors
   glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,-6)
    glVertex2i(-96,6)
    glVertex2i(-102,6)
    glVertex2i(-102,-6)
  glEnd()

end
