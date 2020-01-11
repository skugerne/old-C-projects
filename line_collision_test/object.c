#include "main.h"
#include "object.h"



#define SMALL_MOVEMENT 0.0000000001



object_t *g_objects;
line_t *g_lines;
sector_t g_sectors[NUM_SECTORS][NUM_SECTORS];

// display stuff
bool g_showGrid, g_showCM;
unsigned g_colSearchCalls, g_collCalls;

// world options
bool g_gravBot;
bool g_gravStar;
border_bounce_t g_borderType;

// local global for numbering objects
unsigned id_counter;



void initObjects(){
	g_objects = NULL;
	g_lines = NULL;
	for(int i=0;i<NUM_SECTORS;++i)
		for(int j=0;j<NUM_SECTORS;++j)
			g_sectors[i][j].timestamp = 0;
	
	g_showGrid = false;
	g_showCM = false;
	g_colSearchCalls = 0;
	g_collCalls = 0;
	
	g_gravBot = false;
	g_gravStar = false;
	g_borderType = BORDER_BOUNCE;
	
	srand(0);
	
	for(int i=0;i<NUM_OBJ;++i){
		object_t *oPtr = new object_t;
		initObject_t(oPtr);
		sortObject_t(oPtr);
	}
	
	for(int i=0;i<NUM_LINES;++i){
		line_t *lPtr = new line_t;
		initLine_t(lPtr);
	}
}



void initObject_t(object_t *oPtr){
	oPtr->id = ++id_counter;
	oPtr->alive = true;
	
	oPtr->x = 200.0 + 400.0 * (float)rand() / (RAND_MAX+1.0);
	oPtr->y = 150.0 + 300.0 * (float)rand() / (RAND_MAX+1.0);
	oPtr->dx = -0.125 + 0.25 * (float)rand() / (RAND_MAX+1.0);
	oPtr->dy = -0.125 + 0.25 * (float)rand() / (RAND_MAX+1.0);
	oPtr->radius = 1.0 + 5.0 * (float)rand() / (RAND_MAX+1.0);
	
	int choose = 1 + (int)(8.0 * (float)rand() / (RAND_MAX+1.0));
	if( choose == 1 )
		oPtr->radius *= 2;
	else if( choose > 4 )
		oPtr->radius /= 2;
	
	// shortcut for starting them at large sizes for testing
	oPtr->radius *= BASE_SIZE;
	
	oPtr->inCollision = 0;
	oPtr->prev = NULL;
	oPtr->next = NULL;
}



// put the given object_t into the global list
// note that the grid system doesn't require sorting by size, but still its
//	 handy to have the largest object first when its time to do the star mode
void sortObject_t(object_t *oPtr){
	object_t *leadPtr = g_objects;
	if( leadPtr == NULL || leadPtr->radius < oPtr->radius ){
		g_objects = oPtr;
		oPtr->next = leadPtr;
	}else{
		object_t *followPtr = leadPtr;
		leadPtr = leadPtr->next;
		
		while( leadPtr && leadPtr->radius > oPtr->radius ){
			followPtr = leadPtr;
			leadPtr = leadPtr->next;
		}
		
		followPtr->next = oPtr;
		oPtr->next = leadPtr;
		oPtr->prev = followPtr;
	}
	
	if( oPtr->next )
		oPtr->next->prev = oPtr;
}



void initLine_t(line_t *lPtr){
	object_t *oPtr1 = g_objects;
	object_t *oPtr2 = g_objects;
	int targ;
	
	// find the first end point object
	targ = (int)(NUM_OBJ * (float)rand() / (RAND_MAX+1.0));
	printf("targ 1 is %d\n",targ);
	for(int j=0;j<targ;++j) oPtr1 = oPtr1->next;
	
	// find the second end point object
	targ = (int)(NUM_OBJ * (float)rand() / (RAND_MAX+1.0));
	printf("targ 2 is %d\n",targ);
	for(int j=0;j<targ;++j) oPtr2 = oPtr2->next;
	
	lPtr->obj1 = oPtr1;
	lPtr->obj2 = oPtr2;
	
	lPtr->x = 0.0;
	lPtr->y = 0.0;
	lPtr->angle1 = 0.0;
	lPtr->angle2 = 0.0;
	
	// put this line segment at the head of the lines list
	lPtr->next = g_lines;
	g_lines = lPtr;
}



// increase the radii of all objects
void radiusChange(float mod){
	object_t *oPtr = g_objects;
	while( oPtr ){
		oPtr->radius *= mod;
		oPtr = oPtr->next;
	}
}



// increase the rate of movement of all objects
void speedChange(float mod){
	object_t *oPtr = g_objects;
	while( oPtr ){
		oPtr->dx *= mod;
		oPtr->dy *= mod;
		oPtr = oPtr->next;
	}
}



// use the first element in the list to cancel out the movement of all of them
void cancelMomentum(){
	list_stats_t stats;
	listStats(&stats);
	
	if( g_objects ){
		g_objects->dx -= stats.inertiaX / (g_objects->radius*g_objects->radius);
		g_objects->dy -= stats.inertiaY / (g_objects->radius*g_objects->radius);
	}
}



void setStar(){
	if( !g_objects ) return;
	
	if( g_gravStar )
		g_objects->radius *= 3;
	else
		g_objects->radius /= 3;
}



void drawObject_t(object_t *oPtr){
	glPushMatrix();
	glTranslatef(oPtr->x , oPtr->y , 0.0);
	
	if( g_gravStar && oPtr == g_objects ){
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);
		
		glColor4f(1,1,.5,.5);
		glBegin(GL_TRIANGLE_FAN);
			glVertex2f(0,0);
			for(int i=0;i<STAR_POINTS;i+=2){
				glColor4f(1,1,0,0);
				glVertex2f(
					STAR_GLOW_1 * oPtr->radius * cos( 2 * M_PI * ((float)i / STAR_POINTS) ),
					STAR_GLOW_1 * oPtr->radius * sin( 2 * M_PI * ((float)i / STAR_POINTS) )
				);
				glColor4f(1,.9,.3,.1);
				glVertex2f(
					STAR_GLOW_1 * oPtr->radius * cos( 2 * M_PI * ((float)(i+1) / STAR_POINTS) ),
					STAR_GLOW_1 * oPtr->radius * sin( 2 * M_PI * ((float)(i+1) / STAR_POINTS) )
				);
			}
			glColor4f(1,1,0,0);
			glVertex2f(STAR_GLOW_1 * oPtr->radius,0);
		glEnd();
		
		glBegin(GL_TRIANGLES);
			for(int i=0;i<STAR_POINTS;i+=2){
				glColor4f(1,1,0,0);
				glVertex2f(
					STAR_GLOW_1 * oPtr->radius * cos( 2 * M_PI * ((float)i / STAR_POINTS) ),
					STAR_GLOW_1 * oPtr->radius * sin( 2 * M_PI * ((float)i / STAR_POINTS) )
				);
				glColor4f(1,.9,.3,.1);
				glVertex2f(
					STAR_GLOW_1 * oPtr->radius * cos( 2 * M_PI * ((float)(i+1) / STAR_POINTS) ),
					STAR_GLOW_1 * oPtr->radius * sin( 2 * M_PI * ((float)(i+1) / STAR_POINTS) )
				);
				glColor4f(1,.9,.3,0);
				glVertex2f(
					STAR_GLOW_2 * oPtr->radius * cos( 2 * M_PI * (((float)i+.5) / STAR_POINTS) ),
					STAR_GLOW_2 * oPtr->radius * sin( 2 * M_PI * (((float)i+.5) / STAR_POINTS) )
				);
				glVertex2f(
					STAR_GLOW_2 * oPtr->radius * cos( 2 * M_PI * (((float)i+.5) / STAR_POINTS) ),
					STAR_GLOW_2 * oPtr->radius * sin( 2 * M_PI * (((float)i+.5) / STAR_POINTS) )
				);
				glColor4f(1,1,0,0);
				glVertex2f(
					STAR_GLOW_1 * oPtr->radius * cos( 2 * M_PI * ((float)(i+2) / STAR_POINTS) ),
					STAR_GLOW_1 * oPtr->radius * sin( 2 * M_PI * ((float)(i+2) / STAR_POINTS) )
				);
				glColor4f(1,.9,.3,.1);
				glVertex2f(
					STAR_GLOW_1 * oPtr->radius * cos( 2 * M_PI * ((float)(i+1) / STAR_POINTS) ),
					STAR_GLOW_1 * oPtr->radius * sin( 2 * M_PI * ((float)(i+1) / STAR_POINTS) )
				);
			}
		glEnd();
		
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		
		glColor3f(1,1,1);
		glBegin(GL_TRIANGLE_FAN);
			glVertex2f(0,0);
			glColor3f(1,1,0);
			for(int i=0;i<STAR_SIDES;++i){
				glVertex2f(
					oPtr->radius * cos( 2 * M_PI * ((float)i / STAR_SIDES) ),
					oPtr->radius * sin( 2 * M_PI * ((float)i / STAR_SIDES) )
				);
			}
			glVertex2f(oPtr->radius,0);
		glEnd();
	}else{
		if( oPtr->inCollision + 100 >= g_timestamp )
			glColor3f(1.0,0.0,0.0);
		else
			glColor3f(0.0,1.0,0.0);
		
		glBegin(GL_LINE_LOOP);
			for(int i=0;i<OBJ_SIDES;++i){
				glVertex2f(
					oPtr->radius * cos( 2 * M_PI * ((float)i / OBJ_SIDES) ),
					oPtr->radius * sin( 2 * M_PI * ((float)i / OBJ_SIDES) )
				);
			}
		glEnd();
	}
	
	glPopMatrix();
}




void drawLine_t(line_t *lPtr){
	glColor3f(0.0,0.0,1.0);
	glBegin(GL_LINE_LOOP);
	  glVertex2f(lPtr->obj1->x,lPtr->obj1->y);
	  glVertex2f(lPtr->obj2->x,lPtr->obj2->y);
	glEnd();
}



void updateObject_t(object_t *oPtr){
	
	if( g_borderType == BORDER_BOUNCE ){
		// bounce off the walls
		
		if( oPtr->x - oPtr->radius < 0.0 && oPtr->dx < 0.0 )
			oPtr->dx *= -1;
		else if( oPtr->x + oPtr->radius > 800.0 && oPtr->dx > 0.0 )
			oPtr->dx *= -1;
		if( oPtr->y - oPtr->radius < 0.0 && oPtr->dy < 0.0 )
			oPtr->dy *= -1;
		else if( oPtr->y + oPtr->radius > 600.0 && oPtr->dy > 0.0 )
			oPtr->dy *= -1;
			
	}else if( g_borderType == BORDER_WRAP ){
		// wrap around the sides rather than bounce
		
		if( oPtr->x + oPtr->radius < 0.0 && oPtr->dx < 0.0 )
			oPtr->x += 800.0 + 2*oPtr->radius;
		else if( oPtr->x - oPtr->radius > 800.0 && oPtr->dx > 0.0 )
			oPtr->x -= 800.0 + 2*oPtr->radius;
		if( oPtr->y + oPtr->radius < 0.0 && oPtr->dy < 0.0 )
			oPtr->y += 600.0 + 2*oPtr->radius;
		else if( oPtr->y - oPtr->radius > 600.0 && oPtr->dy > 0.0 )
			oPtr->y -= 600.0 + 2*oPtr->radius;
	}
	
	oPtr->x += oPtr->dx;
	oPtr->y += oPtr->dy;
	
	// apply effects of gravity to bottom of screen if it is active
	if( g_gravBot ) oPtr->dy -= GRAV_BOT_ACC;
	
	// apply effects of star gravity if it is active
	if( g_gravStar && g_objects != oPtr ){
		float xDist = g_objects->x - oPtr->x;
		float yDist = g_objects->y - oPtr->y;
		float dist = xDist * xDist + yDist * yDist;
		
		if( sqrt(dist) > g_objects->radius + oPtr->radius ){
			float bigMass = g_objects->radius * g_objects->radius;
			float lilMass = oPtr->radius * oPtr->radius;
			
			float accelStar = STAR_GRAVITY * lilMass / dist;
			float accelObj = STAR_GRAVITY * bigMass / dist;
			
			float angle = atan( yDist / xDist );
			
			if(xDist < 0){
				oPtr->dx -= accelObj * cos(angle);
				oPtr->dy -= accelObj * sin(angle);
				g_objects->dx += accelStar * cos(angle);
				g_objects->dy += accelStar * sin(angle);
			}else{
				oPtr->dx += accelObj * cos(angle);
				oPtr->dy += accelObj * sin(angle);
				g_objects->dx -= accelStar * cos(angle);
				g_objects->dy -= accelStar * sin(angle);
			}
		}
	}
}



// collect statistics on the list of objects
void listStats(list_stats_t *stats){
	if( !stats ) return;
	
	stats->timestamp = g_timestamp;
	stats->numObj = 0;
	stats->kineticEng = 0.0;
	stats->inertiaX = 0.0;
	stats->inertiaX = 0.0;
	stats->centerMassX = 0.0;
	stats->centerMassY = 0.0;
	stats->mass = 0.0;
	
	object_t *oPtr = g_objects;
	while( oPtr ){
		float mass = oPtr->radius*oPtr->radius;
		
		stats->numObj += 1;
		stats->kineticEng += mass*(oPtr->dx*oPtr->dx + oPtr->dy*oPtr->dy);
		stats->inertiaX += mass*oPtr->dx;
		stats->inertiaY += mass*oPtr->dy;
		stats->centerMassX = (oPtr->x * mass + stats->centerMassX * stats->mass) / (mass + stats->mass);
		stats->centerMassY = (oPtr->y * mass + stats->centerMassY * stats->mass) / (mass + stats->mass);
		stats->mass += mass;
		
		oPtr = oPtr->next;
	}
}     



// handle the interaction of objects with other objects
void collide(object_t *obj1, object_t *obj2){
	++g_collCalls;
	
	if( g_gravStar && obj1 == g_objects ){
		obj2->alive = false;
		obj1->dx += (obj2->dx*obj2->radius*obj2->radius) / (obj1->radius*obj1->radius);
		obj1->dy += (obj2->dy*obj2->radius*obj2->radius) / (obj1->radius*obj1->radius);
		obj1->radius = sqrt(obj1->radius*obj1->radius + obj2->radius+obj2->radius);
		//fprintf(stderr,"Object %u to be destroyed.\n",obj2->id);
		return;
	}
	
	float dx, dy, x, y;
	float dxy, v, v1, v2, nullV;
	float incomingAngle, centersAngle, nullAngle;
	float mass1 = obj1->radius * obj1->radius;
	float mass2 = obj2->radius * obj2->radius;
	
	// make things stationary around the 2nd object to ease calculations
	dx = obj1->dx - obj2->dx;
	dy = obj1->dy - obj2->dy;
	
	// make 2nd object's coordinates 0,0 to ease calculations
	x = obj1->x - obj2->x;
	y = obj1->y - obj2->y;
	
	// not moving relative to each other, therefore cannot be colliding
	if( fabfs(dx) < SMALL_MOVEMENT && fabfs(dy) < SMALL_MOVEMENT ) return;
	
	// speed of closure
	dxy = sqrt(dx * dx + dy * dy);			// closing velocity
	
	// angle that the 1st object is "approaching" from
	incomingAngle = atan(dy/dx);
	if(dx < 0) incomingAngle += M_PI;
	if(incomingAngle < 0) incomingAngle += 2 * M_PI;
	// should result in a 0-360 deg range of angles
	// 270 to 360:	lower right
	// 180 to 270:	lower left
	// 90 to 180:	upper left
	// 0 to 90:		upper right
	
	// angle that the object centers are at
	centersAngle = atan(y/x);
	if(x < 0) centersAngle += M_PI;
	if(centersAngle < 0) centersAngle += 2 * M_PI;
	// should result in a 0-360 deg range of angles
	// 270 to 360:	lower right
	// 180 to 270:	lower left
	// 90 to 180:	upper left
	// 0 to 90:		upper right
	
	// if the angles are within 90 degrees, there can be no colission
	// 180 degree separation means straight-on collision
	if( fabsf(incomingAngle - centersAngle) < M_PI / 2 ) return;
	if( fabsf(incomingAngle - centersAngle) > 3 * M_PI / 2 ) return;
		
	nullAngle = incomingAngle - centersAngle;
	nullV = dxy * sin(nullAngle);		// non-collision v (relative of course)
	v = dxy * cos(nullAngle);			// collision v, head-on
	
	// one-dimentional elastic collision
	// (solutions to cons of momentum and cons of energy)
	v1 = v * ((mass1 - mass2) / (mass1 + mass2));
	v2 = v * ((2 * mass1) / (mass1 + mass2));
	
	obj1->dx = obj2->dx + cos(centersAngle) * v1 - sin(centersAngle) * nullV;
	obj1->dy = obj2->dy + sin(centersAngle) * v1 + cos(centersAngle) * nullV;
	obj2->dx = obj2->dx + cos(centersAngle) * v2;
	obj2->dy = obj2->dy + sin(centersAngle) * v2;
	
	// visually note that a collision occurred
	obj1->inCollision = g_timestamp;
	obj2->inCollision = g_timestamp;
}



static inline float wieghtedAvg(float a, float b, float aDist, float bDist){
  return a + ((a - b) * (aDist + bDist)) / aDist;
}



// handle the interaction of objects with lines
// for the purposes of this function, the point B is the independant obj
// points A and C are the end points of the line
// point P is the point of collision
void collide(object_t *objB, line_t *line){
	++g_collCalls;
	
	object_t *objA = line->obj1;
	object_t *objC = line->obj2;
	
	float dx, dy, x, y;
	float dxy, v, v1, v2, nullV;
	float incomingAngle, centersAngle, nullAngle;
	float mass1 = objB->radius * objB->radius;
	float mass2 = objA->radius * objA->radius + objC>radius * objC->radius;
	float distAB, distAC, distBC, distBP, distAP, distCP;
	float xP, yP, dxP, dyP;
	
	// we will need distances between each of the point pairs
	distAB = dist(objA,objB);
	distBC = dist(objB,objC);
	distAC = dist(objA,objC);
	
	// use pythagorean theorem to throw out cases where object is out
	//   beyond the end points of the line
	// these cases may collide with the end points (object vs object)
	if( distAB * distAB > distBC * distBC + distAC * distAC ) return;
	if( distBC * distBC > distAB * distAB + distAC * distAC ) return;
	
	// use a reformulated law of cosines to find the distance from
	//   the center of the circle to the line
	distBP = distAB * sin(acos((distAC * distAC + distAB * distAB - distBC * distBC) / (2.0 * distAC * distAB)));
	if( distBP > objB->radius ) return;
	
	distAP = sqrt(distAB * distAB - distBP * distBP);
	distCP = distAC - distAP;
	
	xP = wieghtedAvg(objA->x,distAP,objC->x,distCP);
	yP = wieghtedAvg(objA->y,distAP,objC->y,distCP);
	dxP = wieghtedAvg(objA->dx,distAP,objC->dx,distCP);
	dyP = wieghtedAvg(objA->dy,distAP,objC->dy,distCP);
	
	// make things stationary around the point on the line to ease calculations
	dx = objB->dx - dxP;
	dy = objB->dy - dyP;
	
	// make 2nd object's coordinates 0,0 to ease calculations
	x = objB->x - xP;
	y = objB->y - yP;
	
	// not moving relative to each other, therefore cannot be colliding
	if( fabfs(dx) < SMALL_MOVEMENT && fabfs(dy) < SMALL_MOVEMENT ) return;
	
	// speed of closure
	dxy = sqrt(dx * dx + dy * dy);			// closing velocity
	
	// angle at which the force of impact will be applied
	effectAngle = M_PI/2.0 + atan( (objA->y - objC->y)/(objA->x - objC->x) );
	
	return;
	
	// angle that the independent object is "approaching" from
	incomingAngle = atan(dy/dx);
	if(dx < 0) incomingAngle += M_PI;
	if(incomingAngle < 0) incomingAngle += 2 * M_PI;
	// should result in a 0-360 deg range of angles
	// 270 to 360:	lower right
	// 180 to 270:	lower left
	// 90 to 180:	upper left
	// 0 to 90:		upper right
	
	// angle that the object centers are at
	centersAngle = atan(y/x);
	if(x < 0) centersAngle += M_PI;
	if(centersAngle < 0) centersAngle += 2 * M_PI;
	// should result in a 0-360 deg range of angles
	// 270 to 360:	lower right
	// 180 to 270:	lower left
	// 90 to 180:	upper left
	// 0 to 90:		upper right
	
	// if the angles are within 90 degrees, there can be no colission
	// 180 degree separation means straight-on collision
	if( fabsf(incomingAngle - centersAngle) < M_PI / 2 ) return;
	if( fabsf(incomingAngle - centersAngle) > 3 * M_PI / 2 ) return;
		
	nullAngle = incomingAngle - effectAngle;
	nullV = dxy * sin(nullAngle);		// non-collision v (relative of course)
	v = dxy * cos(nullAngle);			// collision v, head-on
	
	// one-dimentional elastic collision
	// (solutions to cons of momentum and cons of energy)
	v1 = v * ((mass1 - mass2) / (mass1 + mass2));
	v2 = v * ((2 * mass1) / (mass1 + mass2));
	
	objB->dx = obj2->dx + cos(centersAngle) * v1 - sin(centersAngle) * nullV;
	objB->dy = obj2->dy + sin(centersAngle) * v1 + cos(centersAngle) * nullV;
	obj2->dx = obj2->dx + cos(centersAngle) * v2;
	obj2->dy = obj2->dy + sin(centersAngle) * v2;
	
	// visually note that a collision occurred
	objB->inCollision = g_timestamp;
}



static inline float dist(object_t *obj1, object_t *obj2){
	float x = obj1->x - obj2->x;
	float y = obj1->y - obj2->y;
	return sqrt(x*x + y*y);
}



// recursive function to search tree for collisions
void colSearchObject_t(object_t *tree, object_t *oPtr){
	++g_colSearchCalls;
	
	bool less = tree->x - oPtr->x < 0;
	
	if( tree->radius + oPtr->radius > fabsf(tree->x - oPtr->x) ){
		if( tree->radius + oPtr->radius > fabsf(tree->y - oPtr->y) )
			if( tree->radius + oPtr->radius > dist(tree,oPtr) )
				collide(tree,oPtr);
		
		if( tree->less )
			colSearchObject_t(tree->less,oPtr);
		if( tree->more )
			colSearchObject_t(tree->more,oPtr);
	}else{
		if( less && tree->less )
			colSearchObject_t(tree->less,oPtr);
		else if( tree->more )
			colSearchObject_t(tree->more,oPtr);
	}
}



// store the object into the sector it falls in
void entreeObject_t(object_t *oPtr, sector_t *sPtr){
	oPtr->less = NULL;
	oPtr->more = NULL;
	
	if( sPtr->timestamp != g_timestamp ){
		// the sector has stale data in it (overwrite it)
		sPtr->tree = oPtr;
		
	}else{
		object_t *nodePtr = sPtr->tree;
		while( 1 ){
			if( nodePtr->x > oPtr->x ){
				if( nodePtr->more ){
					nodePtr = nodePtr->more;
				}else{
					nodePtr->more = oPtr;
					break;
				}
			}else{
				if( nodePtr->less ){
					nodePtr = nodePtr->less;
				}else{
					nodePtr->less = oPtr;
					break;
				}
			}
		}
	}
	
	// mark the sector as having something in it
	sPtr->timestamp = g_timestamp;
}



void engridObject_t(object_t *oPtr){
	// convert x coordinate into a sector index
	int xindex = (int)oPtr->x / SECTOR_SIZE;
	if( xindex < UNDERFLOW_SECTOR ) xindex = UNDERFLOW_SECTOR;
	if( xindex > OVERFLOW_SECTOR ) xindex = OVERFLOW_SECTOR;
	
	// convert y coordinate into a sector index
	int yindex = (int)oPtr->y / SECTOR_SIZE;
	if( yindex < UNDERFLOW_SECTOR ) yindex = UNDERFLOW_SECTOR;
	if( yindex > OVERFLOW_SECTOR ) yindex = OVERFLOW_SECTOR;
	
	// condensed method of safely trying +/- sectors for both x/y coords
	for(int i=-1;i<2;++i)
		if( xindex+i >= UNDERFLOW_SECTOR && xindex+i <= OVERFLOW_SECTOR )
			for(int j=-1;j<2;++j)
				if( yindex+j >= UNDERFLOW_SECTOR && yindex+j <= OVERFLOW_SECTOR )
					if( g_sectors[xindex+i][yindex+j].timestamp == g_timestamp )
						colSearchObject_t(g_sectors[xindex+i][yindex+j].tree,oPtr);
	
	// store the object into the sector it falls in
	entreeObject_t(oPtr,&g_sectors[xindex][yindex]);
}



void objectLineCollision(object_t *oPtr){
	line_t *lPtr = g_lines;
	while( lPtr ){
		
		if( oPtr != lPtr->obj1 && oPtr != lPtr->obj2 ){
			// check for line vs point collision
			//collide(oPtr,lPtr);
		}
		
		lPtr = lPtr->next;
	}
}
