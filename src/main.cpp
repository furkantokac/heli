#include <iostream>
#include <string>

#include <GL/glut.h>


///
/// GLOBAL VARS/FUNCS
///
static bool isZero(const float &val, float sens = 100)
{
        if ((int)(val * sens) == 0)
                return true;

        return false;
}

static float approachToZero(float val, const float &rate, float sens = 100)
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
static const void *FONT1 = GLUT_BITMAP_9_BY_15;
static const void *FONT2 = GLUT_BITMAP_TIMES_ROMAN_24;

static const float ORTHO_RIGHT = 100;
static const float ORTHO_LEFT = 0;
static const float ORTHO_TOP = 100;
static const float ORTHO_BOTTOM = 0;
static const int WINDOW_W = 480;
static const int WINDOW_H = 640;
static const int FPS = 60;

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

class ColliderRects
{
public:
        ColliderRects(float &x, float &y) :
                m_x(x),
                m_y(y)
        {}
private:
        float &m_x;
        float &m_y;
};

class ColliderRect
{
public:
        ColliderRect(float &x, float &y, float &w, float &h) :
                m_x(x),
                m_y(y),
                m_w(w),
                m_h(h)
        {}

        ColliderRect(float x, float y, float w, float h, bool) :
                _x(x),
                _y(y),
                _w(w),
                _h(h),
                m_x(_x),
                m_y(_y),
                m_w(_w),
                m_h(_h)
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

private:
        float _x;
        float _y;
        float _w;
        float _h;

        float &m_x;
        float &m_y;
        float &m_w;
        float &m_h;

};

class Shape
{
public:
        Shape(float &x, float &y, float &w, float &h, bool isMirroredX = false) :
                m_x(x),
                m_y(y),
                m_w(w),
                m_h(h),
                m_scale(1.0),
                m_isMirroredX(isMirroredX),
                m_isMirroredY(false)
        {}

        virtual void draw() = 0;

        //  Getter/Setter
        float x() const
        {
                if (m_isMirroredX) return m_x + m_w;
                return m_x;
        }

        float y() const
        {
                if (m_isMirroredY) return m_y + m_h;
                return m_y;
        }

        float w() const
        {
                if (m_isMirroredX) return m_w * m_scale * -1;
                return m_w * m_scale;
        }

        float h() const
        {
                if (m_isMirroredY) return m_h * m_scale * -1;
                return m_h * m_scale;
        }

        void mirroredX(bool state) { m_isMirroredX = state; }
        void mirroredY(bool state) { m_isMirroredY = state; }

private:
        float &m_x;
        float &m_y;
        float &m_w;
        float &m_h;
        float m_scale;

        bool m_isMirroredX;
        bool m_isMirroredY;
};

class ShapePlane : public Shape
{
public:
        ShapePlane(float &x, float &y, float &w, float &h, bool isMirroredX = false) :
                Shape(x,y,w,h,isMirroredX)
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
        ShapeHeli(float &x, float &y, float &w, float &h, bool isMirroredX = false) :
                Shape(x,y,w,h,isMirroredX)
        {}

        void draw()
        {
                Rect::draw(x(),y(), w(),h(), 0.4,0.8,0.2);
        }
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
                m_collider(m_x, m_y, m_w, m_h),
                m_shape(shape)
        {}

        void fixedUpdate()
        {
                if (m_isMovingX)
                {
                        m_x += m_speedingX;
                        if (m_x > m_maxX) m_x = m_maxX;
                        else if (m_x < m_minX) m_x = m_minX;

                        if (!m_isMovedX) m_speedingX = approachToZero(m_speedingX, m_speed * m_deltaMove);
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
                        if (m_y > m_maxY) m_y = m_maxY;
                        else if (m_y < m_minY) m_y = m_minY;

                        if (!m_isMovedY) m_speedingY = approachToZero(m_speedingY, m_speed * m_deltaMove);
                        m_isMovedY = false;

                        if (isZero(m_speedingY))
                        {
                                m_isMovingY = false;
                                m_speedingY = 0;
                        }
                }
        }

        void update()
        {
                if (m_isMovingX) m_x += m_speed;
                if (m_isMovingY) m_y += m_speed;
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

        bool isColliding(const GameObj &other)
        {
                bool collX = m_x + m_w >= other.x() && other.x() + other.w() >= m_x;
                bool collY = m_y + m_h >= other.y() && other.y() + other.h() >= m_y;

                return collX && collY;
        }

        // Getter/Setter
        float x() const { return m_x; }
        float y() const { return m_y; }
        float w() const { return m_w; }
        float h() const { return m_h; }

        void setSpeed(float speed) { m_speed = speed; }
        void setDeltaMove(float deltaMove) { m_deltaMove = deltaMove; }

protected:
        int m_tag;
        float m_x;
        float m_y;
        float m_w;
        float m_h;

        float m_minX = ORTHO_LEFT - m_w/2;
        float m_maxX = ORTHO_RIGHT + - m_w/2;
        float m_minY = ORTHO_BOTTOM - m_w - 1;
        float m_maxY = ORTHO_TOP + 1;

        float m_speed;
        float m_maxSpeedX;
        float m_maxSpeedY;
        float m_speedingX = 0.0;
        float m_speedingY = 0.0;

        bool m_isMovingX = false;
        bool m_isMovingY = false;

        bool m_isMovedX = false;
        bool m_isMovedY = false;

        float m_deltaMove = 0.5;

        ColliderRect m_collider;
        Shape &m_shape;

        bool m_isColliderActive = true;
};

class Player : public GameObj
{
public:
        Player(float x = 40.0, float y = 19, float w = 24.0, float h = 24.0/2) :
                GameObj(0, x, y, w, h, m_shape),
                m_shape(m_x, m_y, m_w, m_h)
        {}

        void reset()
        {
                m_x = 40.0;
                m_y = ORTHO_BOTTOM + 3;
                m_speedingX = 0.0;
                m_speedingY = 0.0;
        }

        void resetAll()
        {
                m_speed = 0.5;
                m_maxSpeedX = 0.6;
                m_maxSpeedY = 0.5;
                m_deltaMove = 0.5;
                m_health = 3;
                m_score = 0;
                reset();
        }

        void draw()
        {
                m_shape.draw();
        }

        void incSpeed() { m_maxSpeedX += 0.1; m_maxSpeedY += 0.1; m_speed += 0.02; m_deltaMove += 0.15; }
        void incScore() { m_score += 10; }
        void decHealth() { m_health--; }
        void incHealth() { m_health++; }

        // Getter/Setter
        int health() const { return m_health; }
        int score() const { return m_score; }

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
        Heli(bool isLeftToRight, float y = 19.0, float speed = 0.5, float x = 40.0, float w = 25.0, float h = 25.0/3) :
                GameObj(1, x, y, w, h, m_shape),
                m_shape(m_x, m_y, m_w, m_h, !isLeftToRight),
                m_isLeftToRight(isLeftToRight)
        {
                if (m_isLeftToRight)
                {
                        m_speed = speed;
                        m_maxX = ORTHO_RIGHT;
                        m_minX = -9999;
                }
                else
                {
                        m_speed = speed * -1;
                        m_maxX = 9999;
                        m_minX = ORTHO_LEFT - m_w;
                }

                reset();
                m_isMovingX = true;
        }

        void reset()
        {
                int randomNum = rand()%100;
                if (m_isLeftToRight) m_x = ORTHO_LEFT - m_w - randomNum;
                else m_x = ORTHO_RIGHT + m_w + randomNum;
        }

        void resetAll()
        {
                if (m_isLeftToRight)
                {
                        m_speed = 0.5;
                }
                else
                {
                        m_speed = -0.5;
                }

                reset();
        }

        void draw()
        {
                if ( m_x > m_maxX || m_x < m_minX ) reset();
                m_shape.draw();
        }

        void incSpeed()
        {
                if (m_isLeftToRight) m_speed += 0.1;
                else m_speed -= 0.1;
        }

private:
        ShapeHeli m_shape;
        bool m_isLeftToRight; // true:left-to-right | false:right-to-left
};

///////////////////////////////////////////////////////////////
/// GAME
///
static bool isGameOver = false;
static const int HELI_SPACE = 15;
Heli helis[] {
        {false, 40 + 0*HELI_SPACE},
        {true, 40 + 1*HELI_SPACE},
        {false, 40 + 2*HELI_SPACE},
        {true, 40 + 3*HELI_SPACE}
};
Player player;

bool isRightKeyPressed = false;
bool isLeftKeyPressed = false;
bool isUpKeyPressed = false;
bool isDownKeyPressed = false;

void processKey(unsigned char key, int x, int y);
void pressKey(int key, int x, int y);
void releaseKey(int key, int x, int y);
void processKeys(int key, int x, int y);
void renderBitmapString(float x, float y, void *font, const char *string);
void timer(int);

void resetGame()
{
        player.resetAll();
        for (auto &heli : helis) heli.resetAll();
}

void winGame()
{
        player.incScore();
        player.incSpeed();
        for (auto &heli : helis) heli.incSpeed();
        player.reset();
}

void loseGame()
{
        player.decHealth();
        if (player.health() <= 0) isGameOver = true;

        player.reset();
}

void display()
{
        ////// Init
        glClear(GL_COLOR_BUFFER_BIT);


        ////// Update
        if (isGameOver)
        {
                resetGame();

                glColor3f(1.000, 0.000, 0.000);
                renderBitmapString(ORTHO_LEFT, ORTHO_TOP/1.5, (void *) FONT2, " OYUN BITTI!");
                renderBitmapString(ORTHO_LEFT, ORTHO_TOP/1.6, (void *) FONT2, " Yeniden baslamak icin Enter'a basin.");
        }
        else
        {
                if (isRightKeyPressed) player.moveRight();
                if (isLeftKeyPressed) player.moveLeft();
                if (isUpKeyPressed) player.moveUp();
                if (isDownKeyPressed) player.moveDown();

                if (player.y() > ORTHO_TOP) winGame();

                player.fixedUpdate();
                for (auto &heli : helis)
                {
                        if (heli.isColliding(player)) loseGame();
                        heli.update();
                }

                ////// Render
                Rect::draw(0,0, 100,37.5, 0.900,0.870,0.900); // Bottom area

                player.draw();
                for (auto &heli : helis) heli.draw();

                glColor3f(0.000, 0.000, 0.000);
                renderBitmapString(79, 98, (void *) FONT1, "Kalan Hak:");
                renderBitmapString(59, 98, (void *) FONT1, "Puan:");

                glColor3f(1.000, 0.000, 0.000);
                renderBitmapString(98, 98, (void *) FONT1, std::to_string(player.health()).c_str());
                renderBitmapString(69, 98, (void *) FONT1, std::to_string(player.score()).c_str());
        }

        ////// Done
        glFlush();
        glutSwapBuffers();
}

int main(int argc, char *argv[])
{
        srand (time(NULL));

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

        //glutInitWindowPosition(200,20);
        glutInitWindowSize(WINDOW_W,WINDOW_H);
        glutCreateWindow("Heli Game");
        glutDisplayFunc(display);

        glutSpecialFunc(pressKey);
        glutIgnoreKeyRepeat(true);
        glutSpecialUpFunc(releaseKey);
        glutKeyboardFunc(processKey);

        glOrtho(ORTHO_LEFT,ORTHO_RIGHT,ORTHO_BOTTOM,ORTHO_TOP, -1, 1);
        glClearColor(1.0, 1.0,1.0,1);

        glutTimerFunc(1000,timer,0);
        glutMainLoop();

        return 0;
}

void processKey(unsigned char key, int, int)
{
        switch (key) {
                case 13: // ENTER
                        isGameOver = false;
                        break;
                default:
                        break;
        }
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
