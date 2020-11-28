print("Lua is loading!")

function init()
  print("No init in Lua quite yet.")
end

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