#include <iostream>
#include <string>

#include <GL/glut.h>


///
/// GLOBAL VARS/FUNCS
///
static bool isZero(float val, float sens = 100)
{
        if ((int)(val * sens) == 0)
                return true;

        return false;
}

static float approachToZero(float val, float rate, float sens = 100)
{
        if (isZero(val, sens))
        {
                return 0;
        }
        else if (val > 0)
        {
                val -= rate;
                if (val < 0) return 0;
        }
        else if (val < 0)
        {
                val += rate;
                if (val > 0) return 0;
        }

        return val;
}

static const float COLLIDER_COLOR_RED = 0.0;
static const float COLLIDER_COLOR_GREEN = 1.0;
static const float COLLIDER_COLOR_BLUE = 0.0;
static const int FPS = 60;
static const void *FONT = GLUT_BITMAP_9_BY_15;


///////////////////////////////////////////////////////////////
/// CLASSES
///
class Rect
{
public:
        Rect(float x, float y, float w, float h, float r = 0.0, float g = 1.0, float b = 0.0) :
                m_x(x),
                m_y(y),
                m_w(w),
                m_h(h),
                m_r(r),
                m_g(g),
                m_b(b)
        {}

        void draw()
        {
                draw(m_x, m_y, m_w, m_h, m_r, m_g, m_b);
        }

        void static draw(float x, float y, float w, float h, float r, float g, float b)
        {
                glColor3f(r,g,b);
                glBegin(GL_POLYGON);
                glVertex2f(x,y);        // Sol alt
                glVertex2f(x,y+h);      // Sol ust
                glVertex2f(x+w,y+h);    // Sag ust
                glVertex2f(x+w,y);      // Sag alt
                glEnd();
        }

        float m_x; // sol
        float m_y; // alt

        float m_w; // width
        float m_h; // height

        float m_r; // red
        float m_g; // green
        float m_b; // blue
};

class Shape
{
public:
        Shape(float &x, float &y, float &w, float &h, float scale = 1.0) :
                m_x(x),
                m_y(y),
                m_w(w),
                m_h(h),
                m_scale(scale)
        {}

        virtual void draw() = 0;

        //  Getter/Setter
        float x() { return m_x; }
        float y() { return m_y; }
        float w() { return m_w * m_scale; }
        float h() { return m_h * m_scale; }

private:
        float &m_x;
        float &m_y;
        float &m_w;
        float &m_h;
        float m_scale;
};

class ShapePlane : public Shape
{
public:
        ShapePlane(float &x, float &y, float &w, float &h, float scale = 1.0) :
                Shape(x,y,w,h,scale)
        {}

        void draw()
        {
                glColor3f(0.27,0.45,0.77);
                glLineWidth(5);
                // Center
                glBegin(GL_LINES);
                glVertex2f(x()+w()/2, y());
                glVertex2f(x()+w()/2, y()+h());
                glEnd();
                // Bottom
                glBegin(GL_LINES);
                glVertex2f(x()+w()/4, y()+h()*2/10);
                glVertex2f(x()+w()-w()/4, y()+h()*2/10);
                glEnd();
                // Top
                glBegin(GL_LINES);
                glVertex2f(x(), y()+h()*7/10);
                glVertex2f(x()+w(), y()+h()*7/10);
                glEnd();
        }
};

class ShapeHeli : public Shape
{
public:
        ShapeHeli(float &x, float &y, float &w, float &h, float scale = 1.0) :
                Shape(x,y,w,h,scale)
        {}

        void draw()
        {
                Rect::draw(x(),y(), w(),h(), 0.27,0.45,0.77);
        }
};

class ColliderRect
{
public:
        ColliderRect(float &x, float &y, float &w, float &h, bool) :
                m_x(x),
                m_y(y),
                m_w(w),
                m_h(h)
        {}

        ColliderRect(float x, float y, float w, float h) :
                this_x(x),
                this_y(y),
                this_w(w),
                this_h(h),
                m_x(this_x),
                m_y(this_y),
                m_w(this_w),
                m_h(this_h)
        {}

        void draw()
        {
                Rect::draw(m_x, m_y, m_w, m_h, COLLIDER_COLOR_RED, COLLIDER_COLOR_GREEN, COLLIDER_COLOR_BLUE);
        }

        bool isColliding(const ColliderRect *otherObject)
        {
                otherObject = nullptr;
                return false;
        }

        bool isColliding(const float x, const float y)
        {
                return x == y;
        }

private:
        float this_x;
        float this_y;
        float this_w;
        float this_h;

        float &m_x;
        float &m_y;
        float &m_w;
        float &m_h;

};

class GameObj
{
public:
        GameObj(int tag, float x, float y, float w, float h, Shape &shape, float speed = 0.1, float maxSpeedX = 0.6, float maxSpeedY = 0.5) :
                m_tag(tag),
                m_x(x),
                m_y(y),
                m_w(w),
                m_h(h),
                m_speed(speed),
                m_maxSpeedX(maxSpeedX),
                m_maxSpeedY(maxSpeedY),
                m_collider(m_x, m_y, m_w, m_h, true),
                m_shape(shape)
        {}

        void updatePosition()
        {
                if (m_isMovingX)
                {
                        m_x += m_speedingX;

                        if (!m_isMovedX) m_speedingX = approachToZero(m_speedingX, m_speed*0.3);
                        m_isMovedX = false;

                        if (isZero(m_speedingX))
                        {
                                m_isMovingX = false;
                                m_speedingX = 0;
                        }
                }

                if (m_isMovingY)
                {
                        m_y += m_speedingY;

                        if (!m_isMovedY) m_speedingY = approachToZero(m_speedingY, m_speed*0.3);
                        m_isMovedY = false;

                        if (isZero(m_speedingY))
                        {
                                m_isMovingY = false;
                                m_speedingY = 0;
                        }
                }
        }

        void moveRight()
        {
                m_isMovingX = m_isMovedX = true;
                m_speedingX += m_speed;
                if (m_speedingX > m_maxSpeedX) m_speedingX = m_maxSpeedX;
        }

        void moveLeft()
        {
                m_isMovingX = m_isMovedX = true;
                m_speedingX -= m_speed;
                if (m_speedingX < -1*m_maxSpeedX) m_speedingX = -1*m_maxSpeedX;
        }

        void moveUp()
        {
                m_isMovingY = m_isMovedY = true;
                m_speedingY += m_speed;
                if (m_speedingY > m_maxSpeedY) m_speedingY = m_maxSpeedY;
        }

        void moveDown()
        {
                m_isMovingY = m_isMovedY = true;
                m_speedingY -= m_speed;
                if (m_speedingY < -1*m_maxSpeedY) m_speedingY = -1*m_maxSpeedY;
        }

        void moveRight(float speed) { m_speedingX += speed; }
        void moveLeft(float speed) { m_speedingX -= speed; }
        void moveUp(float speed) { m_speedingY += speed; }
        void moveDown(float speed) { m_speedingY -= speed; }

        bool isColliderActive(){ return m_isColliderActive; }
        void enableCollider() { m_isColliderActive = true; }
        void disableCollider() { m_isColliderActive = false; }

        // Getter/Setter
        int tag() { return m_tag; }
        float x() { return m_x; }
        float y() { return m_y; }
        float w() { return m_w; }
        float h() { return m_h; }
        float speed() { return m_speed; }
        const ColliderRect& collider() { return m_collider; }
        const Shape& shape() { return m_shape; }

        void setTag(int tag) { m_tag = tag; }
        void setX(float x) { m_x = x; }
        void setY(float y) { m_y = y; }
        void setW(float w) { m_w = w; }
        void setH(float h) { m_h = h; }
        void setSpeed(float speed) { m_speed = speed; }

protected:
        int m_tag;
        float m_x;
        float m_y;
        float m_w;
        float m_h;

        float m_speed;
        float m_maxSpeedX;
        float m_maxSpeedY;
        float m_speedingX = 0.0;
        float m_speedingY = 0.0;

        bool m_isMovingX = false;
        bool m_isMovingY = false;
        bool m_isMovedX = false;
        bool m_isMovedY = false;

        ColliderRect m_collider;
        Shape &m_shape;

        bool m_isColliderActive = true;
};

class Player : public GameObj
{
public:
        Player(float x = 40.0, float y = 19.0, float w = 26.0, float h = 26.0/2) :
                GameObj(0, x, y, w, h, m_shape),
                m_shape(m_x, m_y, m_w, m_h)
        {}

        void draw()
        {
                //m_collider.draw();
                m_shape.draw();
        }

        void decHealth() { m_health--; }
        void incHealth() { m_health++; }

        // Getter/Setter
        int health() { return m_health; }
        int score() { return m_score; }

        void setHealth(int newHealth) { m_health = newHealth; }
        void setScore(int newScore) { m_score = newScore; }

private:
        ShapePlane m_shape;

        int m_health = 3;
        int m_score = 0;
};

class Heli : public GameObj
{
public:
        Heli(float x = 40.0, float y = 19.0, float w = 30.0, float h = 30.0/3) :
                GameObj(0, x, y, w, h, m_shape),
                m_shape(m_x, m_y, m_w, m_h)
        {}

        void draw()
        {
                m_collider.draw();
                m_shape.draw();
        }

        // Getter/Setter

private:
        ShapeHeli m_shape;
};

///////////////////////////////////////////////////////////////
/// GAME
///
/// HELI
// 40 ile 96 arasina gelecekler
// Yani her birine 14 luk alan dusuyor
// Her birinin yuksekligi 12 olacak
// Aralarindaki uzaklik 2 olacak
static const int HELI_SPACE = 15;

Player player;
Heli helis[] {
        {0, 40 + HELI_SPACE * 0},
        {0, 40 + HELI_SPACE * 1},
        {0, 40 + HELI_SPACE * 2},
        {0, 40 + HELI_SPACE * 3}
};

bool isRightKeyPressed = false;
bool isLeftKeyPressed = false;
bool isUpKeyPressed = false;
bool isDownKeyPressed = false;

void pressKey(int key, int xx, int yy);
void releaseKey(int key, int x, int y);
void processKeys(int key, int x, int y);
void renderBitmapString(float x, float y, void *font, const char *string);
void timer(int);

void display()
{
        glClear(GL_COLOR_BUFFER_BIT);


        ////// Bottom Player Area
        Rect::draw(0,0, 100,37.5, 0.900,0.870,0.900);


        ////// Player
        player.draw();
        if (isRightKeyPressed) player.moveRight();
        if (isLeftKeyPressed) player.moveLeft();
        if (isUpKeyPressed) player.moveUp();
        if (isDownKeyPressed) player.moveDown();
        player.updatePosition();


        ////// Heli
        for (auto heli : helis)
        {
                heli.draw();
        }


        ////// UI Texts
        glColor3f(0.000, 0.000, 0.000);
        renderBitmapString(79, 98, (void *) FONT, "Kalan Hak:");
        renderBitmapString(59, 98, (void *) FONT, "Puan:");

        glColor3f(1.000, 0.000, 0.000);
        renderBitmapString(98, 98, (void *) FONT, std::to_string(player.health()).c_str());
        renderBitmapString(69, 98, (void *) FONT, std::to_string(player.score()).c_str());


        glFlush();
        glutSwapBuffers();
}

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

        //glutInitWindowPosition(200,20);
        glutInitWindowSize(480,640);
        glutCreateWindow("Heli Game");
        glutDisplayFunc(display);

        //glutSpecialFunc(processKeys);
        glutSpecialFunc(pressKey);
        glutIgnoreKeyRepeat(true);
        glutSpecialUpFunc(releaseKey);

        glOrtho(0,100,0,100,-1,1);
        glClearColor(1.0, 1.0,1.0,1);

        glutTimerFunc(1000,timer,0);
        glutMainLoop();

        return 0;
}

void pressKey(int key, int, int)
{
        switch (key)
        {
                case GLUT_KEY_UP: isUpKeyPressed = true; break;
                case GLUT_KEY_DOWN: isDownKeyPressed = true; break;
                case GLUT_KEY_RIGHT: isRightKeyPressed = true; break;
                case GLUT_KEY_LEFT: isLeftKeyPressed = true; break;
        }
}

void releaseKey(int key, int, int)
{
        switch (key)
        {
                case GLUT_KEY_UP: isUpKeyPressed = false; break;
                case GLUT_KEY_DOWN: isDownKeyPressed = false; break;
                case GLUT_KEY_RIGHT: isRightKeyPressed = false; break;
                case GLUT_KEY_LEFT: isLeftKeyPressed = false; break;
        }
}

void processKeys(int key, int, int)
{
        switch (key) {
                case GLUT_KEY_RIGHT: break;
                case GLUT_KEY_LEFT: break;
                case GLUT_KEY_UP: break;
                case GLUT_KEY_DOWN: break;
                default: break;
        }
}

void timer(int)
{
        glutPostRedisplay();
        glutTimerFunc(1000/FPS, timer, 0);
}

void renderBitmapString(float x, float y, void *font, const char *string)
{
        const char *c;
        glRasterPos2f(x, y);
        for (c = string; *c != '\0'; c++)
        {
                glutBitmapCharacter(font, *c);
        }
}
