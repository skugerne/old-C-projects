print("Lua is loading!")

radius = 20;   -- how large the collision circle and shield are

mass = 8;   -- how heavy in collisions

-- each pre-scaled for DT of 1/1000
turnRate = .075;       -- how fast it turns .... 0.075 = 75 deg/S
enginePower = .00125;  -- how fast it accelerates .... 0.00125 = 1250 PPS^2
maxSpeed = 1.0;        -- some kind of max speed ... 1.0 = 1000 PPS

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
    firingMod = 30,
    firingPoint = 6,
    x = -10,   -- position on the model
    y = 40
  },
  {
    firingMod = 30,
    firingPoint = 12,
    x = -10,
    y = -40
  },
  {
    firingMod = 4,
    firingPoint = 2,
    x = -20,
    y = 0
  }
}



function draw()
  glColor3f(0,0.3,1)
  glBeginLines()

    -- left front
    glVertex2i(12,0)
    glVertex2i(-2,-7)
    
    -- right front
    glVertex2i(12,0)
    glVertex2i(-2,7)

    --middle line
    glVertex2i(12,0)
    glVertex2i(-12,0)
 
    -- left edge
    glVertex2i(1,-7)
    glVertex2i(-11,-7)

    --right edge
    glVertex2i(1,7)
    glVertex2i(-11,7)

    --right middle
    glVertex2i(-12,2)
    glVertex2i(-2,2)

    --left middle
    glVertex2i(-12,-2)
    glVertex2i(-2,-2)

    --bottom
    glVertex2i(-12,-2)
    glVertex2i(-12,2)

    --bottom wing reght
    glVertex2i(-11,2)
    glVertex2i(-11,7)

    --bottom wing reght
    glVertex2i(-11,-2)
    glVertex2i(-11,-7)

    --middle kanon
    glColor3f(1,1,1)  -- white
    glVertex2i(14,0)
    glVertex2i(12,0)
    glVertex2i(3,7)
    glVertex2i(1,7)
    glVertex2i(3,-7)
    glVertex2i(1,-7)

  glEnd()
end
