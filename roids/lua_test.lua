print("Lua is loading!")

function draw()
  glColor3f(0,0.3,1)
  glBegin()
    glVertex2i(12,0)
    glVertex2i(-2,-7)
    glVertex2i(12,0)
    glVertex2i(-2,7)
    glVertex2i(12,0)
    glVertex2i(-12,0)
 
    -- sideways line
    glVertex2i(-2,-7)
    glVertex2i(-2,7)
  glEnd()
end