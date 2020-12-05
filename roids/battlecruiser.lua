print("Lua is loading!")

require "math"
require "string"

radius = 150;   -- how large the collision circle and shield are

mass = 100;   -- how heavy in collisions

-- each pre-scaled for DT of 1/1000
turnRate = .075;       -- how fast it turns .... 0.075 = 75 deg/S
enginePower = .000125;  -- how fast it accelerates .... 0.00125 = 1250 PPS^2
maxSpeed = 1.0;        -- some kind of max speed ... 1.0 = 1000 PPS

shieldPoints = 100;

weapons = {
  {
    x = 140,         -- position on the model
    y = 0,
    angle = 0,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 20,   -- compared to 1000 updates per second
    glowLimit = 30,
    glowCoolRate = 0.75,   -- shots per second
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_DOUBLE',
    shotmod = 'SHOT_FAST_LONG'
  },
  {
    x = 140,         -- position on the model
    y = -10,
    angle = 30,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 80,   -- compared to 1000 updates per second
    glowLimit = 20,
    glowCoolRate = 0.5,   -- shots per second
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_SINGLE',
    shotmod = 'SHOT_FAST_LONG'
  },
  {
    x = 140,         -- position on the model
    y = 10,
    angle = -30,     -- relative to the model
    xOffset = 0,   -- additional offset to apply from x,y after rotation (if any)
    yOffset = 0,
    fireDelay = 80,   -- compared to 1000 updates per second
    glowLimit = 20,
    glowCoolRate = 0.5,   -- shots per second
    shotname = 'SHOT_WEAK',
    shotcount = 'SHOT_SINGLE',
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

function draw(timestamp)

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

  -- little flashing thing
  if math.floor(timestamp / 250) % 2 == 0 then
    glColor3f(0,0,1)
  else
    glColor3f(0,1,0)
  end
  glBeginPolygon()
    glVertex2i(85,-3)
    glVertex2i(85,3)
    glVertex2i(90,0)
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

   --middle motor
   glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,-4)
    glVertex2i(-96,4)
    glColor3f(0.3,0.3,0.3)
    glVertex2i(-102,6)
    glVertex2i(-102,-6)
  glEnd()

  --left motor
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,-28)
    glVertex2i(-96,-20)
    glColor3f(0.3,0.3,0.3)
    glVertex2i(-102,-18)
    glVertex2i(-102,-30)
  glEnd()

  --right motor
  glColor3f(1,1,1)
  glBeginPolygon()
    glVertex2i(-96,28)
    glVertex2i(-96,20)
    glColor3f(0.3,0.3,0.3)
    glVertex2i(-102,18)
    glVertex2i(-102,30)
  glEnd()

end



-- recursively dump a table
-- from https://stackoverflow.com/questions/9168058/how-to-dump-a-table-to-console
function dump(o)
  if type(o) == 'table' then
     local s = '{ '
     for k,v in pairs(o) do
        if type(k) ~= 'number' then k = '"'..k..'"' end
        s = s .. '['..k..'] = ' .. dump(v) .. ','
     end
     return s .. '} '
  else
     return tostring(o)
  end
end



function aiUpdate(world)
  print(
    string.format(
      "AI update in Lua ... myself (%0.02f,%0.02f) ... star (%0.02f,%0.02f).",
      world.myself.x,
      world.myself.y,
      world.massiveObjects[1].x,
      world.massiveObjects[1].y
    )
  )
  findHot(1,2,6)
end