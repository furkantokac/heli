#include <iostream>
#include <vector>
#include <string>

#include <GL/glut.h>

using namespace std;

///////////////////////////////////////////////////////////////
/// DOC
///
/// ### Aliases
///
/// x   : coordinate x
/// y   : coordinate y
/// w   : width
/// h   : height
/// c   : coordinates, means c.x c.y
/// d   : dimensions, means d.w x.h
/// cd : coordinates dimensions, means cd.x cd.y cd.w cd.h
/// RGB : red, green, blue colors
/// m_  : member variable of the class
///
/// ### Position system of graphics
///
/// When x and y is used for a graphic, it means left, bottom part of the graphic, like this:
///
/// ****
/// *  *
/// O***


///////////////////////////////////////////////////////////////
/// GLOBALS
struct RGB {
        float r;
        float g;
        float b;

        RGB operator+(const RGB &other) const { return {r+other.r, g+other.g, b+other.b}; }
        RGB operator-(const RGB &other) const { return {r-other.r, g-other.g, b-other.b}; }
};

struct C {
        float x = 0;
        float y = 0;
};

struct D {
        float w = 0;
        float h = 0;
};

struct CD {
        float x = 0;
        float y = 0;
        float w = 0;
        float h = 0;

        CD operator+(const C &other) const { return {x+other.x, y+other.y, w, h}; }
        C c() const { return {x, y}; }
};

struct Cbool {
        bool x = false;
        bool y = false;
};

static const RGB COLLIDER_COLOR = {0.0, 1.0, 0.0};
static const void *FONT1 = GLUT_BITMAP_9_BY_15;
static const void *FONT2 = GLUT_BITMAP_TIMES_ROMAN_24;

static const float ORTHO_RIGHT = 100;
static const float ORTHO_LEFT = 0;
static const float ORTHO_TOP = 100;
static const float ORTHO_BOTTOM = 0;
static const int WINDOW_W = 480;
static const int WINDOW_H = 640;
static const int FPS = 60;

static const int HELI_RANDOMNESS = 60;
static const int HELI_SPACE = 15;

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

static bool isRectsColliding(const CD &rect1, const CD &rect2)
{
        bool collX = rect1.x + rect1.w >= rect2.x && rect2.x + rect2.w >= rect1.x;
        bool collY = rect1.y + rect1.h >= rect2.y && rect2.y + rect2.h >= rect1.y;

        return collX && collY;
}

///////////////////////////////////////////////////////////////
/// CLASSES
///
class Rect
{
public:
        Rect(CD &cd, RGB rgb = {0.0, 1.0, 0.0}) :
                m_cd(cd),
                m_rgb(rgb)
        {}

        void draw()
        {
                draw(m_cd, m_rgb);
        }

        void static setColor(const RGB &rgb)
        {
                glColor3f(rgb.r, rgb.g, rgb.b);
        }

        void static draw(const CD &cd, const RGB &rgb)
        {
                glColor3f(rgb.r, rgb.g, rgb.b);
                draw(cd);
        }

        void static draw(const CD &cd)
        {
                glBegin(GL_POLYGON);
                glVertex2f(cd.x, cd.y);           // left bottom
                glVertex2f(cd.x, cd.y+cd.h);      // left top
                glVertex2f(cd.x+cd.w, cd.y+cd.h); // right top
                glVertex2f(cd.x+cd.w, cd.y);      // right bottom
                glEnd();
        }

private:
        CD m_cd;
        RGB m_rgb;
};


class Shape
{
public:
        Shape(CD &cd, Cbool isMirrored) :
                m_cd(cd),
                m_scale(1.0),
                m_isMirroredX(isMirrored.x),
                m_isMirroredY(isMirrored.y)
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
        CD &m_cd;
        float &m_x = m_cd.x;
        float &m_y = m_cd.y;
        float &m_w = m_cd.w;
        float &m_h = m_cd.h;
        float m_scale;

        bool m_isMirroredX;
        bool m_isMirroredY;
};

class ShapePlane : public Shape
{
public:
        ShapePlane(CD &cd, Cbool isMirrored = {false, false}) :
                Shape(cd, isMirrored)
        {}

        void draw()
        {
                glColor3f(0.27,0.45,0.77);

                glLineWidth(5);
                // Center
                Rect::draw({x() + w()/2 - w()/40,
                            y() + 0,
                            w()/20,
                            h()});
                // Bottom
                Rect::draw({x() + w()/4,
                            y() + h()*2/10,
                            w() - w()/4*2,
                            h()/20});
                // Top
                Rect::draw({x() + 0,
                            y() + h()*7/10,
                            w(),
                            h()/20});
        }
};

class ShapeHeli : public Shape
{
public:
        ShapeHeli(CD &cd, RGB &rgb, Cbool isMirrored = {false, false}) :
                Shape(cd,isMirrored),
                m_rgb(rgb)
        {}

        void draw()
        {
                // Tail
                Rect::setColor(m_rgb - RGB({0.2,0.2,0.2}));
                Rect::draw({x() + 0,
                            y() + h()*5/10 - h()*3/20/2,
                            w()*5/10,
                            h()*3/20});

                // Top Box
                Rect::draw({x() + w()*7/10,
                            y() + h(),
                            w()*1/10,
                            h()*3/20});

                // Body
                Rect::setColor(m_rgb);
                Rect::draw({x() + w()*5/10,
                            y() + h()*5/10 - h()*10/10/2,
                            w()*5/10,
                            h()*10/10});

                // Window
                Rect::setColor({0.85, 0.89, 0.95});
                Rect::draw({x() + w()*31/40,
                            y() + h()/2,
                            w()*2/10,
                            h()*3/10});

                // Bottom - Left
                Rect::setColor({0, 0, 0});
                Rect::draw({x() + w()*9/10,
                            y() - h()*2/10,
                            w()*1/25,
                            h()*3/10});

                // Bottom - Right
                Rect::draw({x() + w()*6/10,
                            y() - h()*2/10,
                            w()*1/25,
                            h()*3/10});

                // Bottom - Bottom
                Rect::draw({x() + w()*5/10,
                            y() - h()*2/10,
                            w()*5/10,
                            h()*1/10});

                // Propeller Top
                glLineWidth(2);
                glBegin(GL_LINES);
                glVertex2f(x() + w()*3/10, y() + h() + h()*1/10);
                glVertex2f(x() + w()*13/10, y() + h() + h()*2/10);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(x() + w()*3/10, y() + h() + h()*2/10);
                glVertex2f(x() + w()*13/10, y() + h() + h()*1/10);
                glEnd();

                // Propeller Back
                glBegin(GL_LINES);
                glVertex2f(x() + w()*-1/10, y() + h()*5/10);
                glVertex2f(x() + w()*1/10, y() + h()*6/10);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(x() + w()*-1/10, y() + h()*6/10);
                glVertex2f(x() + w()*1/10, y() + h()*5/10);
                glEnd();
        }

private:
        RGB &m_rgb;
};

class ColliderRect
{
public:
        ColliderRect(CD cd, CD &cdParent) :
                m_cd(cd),
                m_cdParent(cdParent)
        {}

        void draw()
        {
                Rect::draw(cdGlobal(), COLLIDER_COLOR);
        }

        bool isColliding(const ColliderRect &other) const
        {
                return isRectsColliding(cdGlobal(), other.cdGlobal());
        }

        // Getter/Setter
        CD cd() const { return m_cd; }
        CD cdGlobal() const { return m_cd + m_cdParent.c(); }

private:
        CD m_cd;
        CD &m_cdParent;
};

class ColliderRects
{
public:
        ColliderRects(CD &cd, vector<ColliderRect> colliders = {}) :
                m_cd(cd),
                m_colliders(colliders)
        {}

        void draw()
        {
                for (auto &colliderRect : m_colliders)
                {
                        colliderRect.draw();
                }
        }

        void defineCollider(CD cd)
        {
                m_colliders.push_back({cd, m_cd});
        }

        bool isColliding(const ColliderRects &colliderRects) const
        {
                return isColliding(colliderRects.colliders());
        }

        bool isColliding(const vector<ColliderRect> &colliders) const
        {
                for (auto &collider1 : colliders)
                {
                        for (auto &collider2 : m_colliders)
                        {
                                if (collider1.isColliding(collider2))
                                {
                                        return true;
                                }
                        }
                }

                return false;
        }

        // Getter/Setter
        const vector<ColliderRect> &colliders() const { return m_colliders; }

private:
        CD &m_cd;
        vector<ColliderRect> m_colliders;
};

class ColliderRectsPlane : public ColliderRects
{
public:
        ColliderRectsPlane(CD &cd) :
                ColliderRects(cd)
        {
                // Center
                defineCollider({cd.w/2 - cd.w/40,
                                0,
                                cd.w/20,
                                cd.h});
                // Bottom
                defineCollider({cd.w/4,
                                cd.h*2/10,
                                cd.w - cd.w/4*2,
                                cd.h/20});
                // Top
                defineCollider({0,
                                cd.h*7/10,
                                cd.w,
                                cd.h/20});
        }
};

class ColliderRectsHeli : public ColliderRects
{
public:
        ColliderRectsHeli(CD &cd, bool isLeftToRight = true) :
                ColliderRects(cd)
        {
                if (isLeftToRight)
                {
                        // Body
                        defineCollider({cd.w*5/10,
                                        cd.h*5/10 - cd.h*14/10/2,
                                        cd.w*5/10,
                                        cd.h*14/10});

                        // Tail
                        defineCollider({-2,
                                        cd.h*5/10 - cd.h*3/20/2,
                                        cd.w*6/10,
                                        cd.h*3/20});
                }
                else
                {
                        // Body
                        defineCollider({0,
                                        cd.h*5/10 - cd.h*14/10/2,
                                        cd.w*5/10,
                                        cd.h*14/10});

                        // Tail
                        defineCollider({cd.w*5/10,
                                        cd.h*5/10 - cd.h*3/20/2,
                                        cd.w*6/10,
                                        cd.h*3/20});
                }
        }
};

class GameObj
{
public:
        GameObj(int tag, CD cd, Shape &shape, ColliderRects &colliderRects) :
                m_tag(tag),
                m_cd(cd),
                m_shape(shape),
                m_colliderRects(colliderRects)
        {}

        void fixedUpdate()
        {
                if (m_isMoving.x)
                {
                        m_x += m_speeding.x;
                        if (m_x > m_maxX) m_x = m_maxX;
                        else if (m_x < m_minX) m_x = m_minX;

                        if (!m_isMoved.x) m_speeding.x = approachToZero(m_speeding.x, m_speed * m_deltaMove);
                        m_isMoved.x = false;

                        if (isZero(m_speeding.x))
                        {
                                m_isMoving.x = false;
                                m_speeding.x = 0;
                        }
                }

                if (m_isMoving.y)
                {
                        m_y += m_speeding.y;
                        if (m_y > m_maxY) m_y = m_maxY;
                        else if (m_y < m_minY) m_y = m_minY;

                        if (!m_isMoved.y) m_speeding.y = approachToZero(m_speeding.y, m_speed * m_deltaMove);
                        m_isMoved.y = false;

                        if (isZero(m_speeding.y))
                        {
                                m_isMoving.y = false;
                                m_speeding.y = 0;
                        }
                }
        }

        void update()
        {
                if (m_isMoving.x) m_x += m_speed;
                if (m_isMoving.y) m_y += m_speed;
        }

        void moveRight()
        {
                m_isMoving.x = m_isMoved.x = true;
                m_speeding.x += m_speed;
                if (m_speeding.x > m_maxSpeed.x) m_speeding.x = m_maxSpeed.x;
        }

        void moveLeft()
        {
                m_isMoving.x = m_isMoved.x = true;
                m_speeding.x -= m_speed;
                if (m_speeding.x < -1*m_maxSpeed.x) m_speeding.x = -1*m_maxSpeed.x;
        }

        void moveUp()
        {
                m_isMoving.y = m_isMoved.y = true;
                m_speeding.y += m_speed;
                if (m_speeding.y > m_maxSpeed.y) m_speeding.y = m_maxSpeed.y;
        }

        void moveDown()
        {
                m_isMoving.y = m_isMoved.y = true;
                m_speeding.y -= m_speed;
                if (m_speeding.y < -1*m_maxSpeed.y) m_speeding.y = -1*m_maxSpeed.y;
        }

        void moveRight(float speed) { m_speeding.x += speed; }
        void moveLeft(float speed) { m_speeding.x -= speed; }
        void moveUp(float speed) { m_speeding.y += speed; }
        void moveDown(float speed) { m_speeding.y -= speed; }

        bool isColliding(const GameObj &other) const
        {
                return m_colliderRects.isColliding(other.colliderRects());
        }

        // Getter/Setter
        float x() const { return m_x; }
        float y() const { return m_y; }
        float w() const { return m_w; }
        float h() const { return m_h; }
        ColliderRects &colliderRects() const { return m_colliderRects; }

        void setSpeed(float speed) { m_speed = speed; }
        void setMaxSpeed(C maxSpeed) { m_maxSpeed = maxSpeed; }
        void setDeltaMove(float deltaMove) { m_deltaMove = deltaMove; }


protected:
        int m_tag;
        CD m_cd;
        float &m_x = m_cd.x;
        float &m_y = m_cd.y;
        float &m_w = m_cd.w;
        float &m_h = m_cd.h;

        C m_minC = {ORTHO_LEFT - m_w/2, ORTHO_BOTTOM - m_w - 1};
        float &m_minX = m_minC.x;
        float &m_minY = m_minC.y;

        C m_maxC = {ORTHO_RIGHT - m_w/2, ORTHO_TOP + 1};
        float &m_maxX = m_maxC.x;
        float &m_maxY = m_maxC.y;

        float m_speed = 0.1;
        C m_maxSpeed = {0.6, 0.5};
        C m_speeding = {0.0, 0.0};

        Cbool m_isMoving = {false, false};
        Cbool m_isMoved = {false, false};

        float m_deltaMove = 0.5;

        Shape &m_shape;
        ColliderRects &m_colliderRects;

        bool m_isColliderActive = true;
};

class Player : public GameObj
{
public:
        Player(CD cd = {40.0, 19.0, 24.0, 24.0/2}) :
                GameObj(0, cd, m_shape, m_colliderRects),
                m_shape(m_cd),
                m_colliderRects(m_cd)
        {}

        void reset()
        {
                m_x = 40.0;
                m_y = ORTHO_BOTTOM + 3;
                m_speeding.x = 0.0;
                m_speeding.y = 0.0;
        }

        void resetAll()
        {
                m_speed = 0.5;
                m_maxSpeed = {0.6, 0.5};
                m_deltaMove = 0.5;
                m_health = 3;
                m_score = 0;
                reset();
        }

        void draw()
        {
                m_colliderRects.draw();
                m_shape.draw();
        }

        void incSpeed() { m_maxSpeed.x += 0.1; m_maxSpeed.y += 0.1; m_speed += 0.02; m_deltaMove += 0.15; }
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
        ColliderRectsPlane m_colliderRects;

        int m_health = 3;
        int m_score = 0;
};

class Heli : public GameObj
{
public:
        Heli(bool isLeftToRight, RGB rgb, float y = 19.0, float speed = 0.5, float x = 40.0, float w = 25.0, float h = 25.0/3.5) :
                GameObj(1, {x, y, w, h}, m_shape, m_colliderRects),
                m_rgb(rgb),
                m_shape(m_cd, m_rgb, {!isLeftToRight, false}),
                m_colliderRects(m_cd, isLeftToRight),
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
                m_isMoving.x = true;
        }

        void reset()
        {
                int randomNum = rand()%HELI_RANDOMNESS;
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
        RGB m_rgb;
        ShapeHeli m_shape;
        ColliderRectsHeli m_colliderRects;

        bool m_isLeftToRight; // true:left-to-right | false:right-to-left
};

///////////////////////////////////////////////////////////////
/// GAME
///
static bool isGameOver = false;
Heli helis[] {
        {false, {1.0, 0.75, 0.0}, 40 + 0*HELI_SPACE},
        {true, {0.93, 0.49, 0.19}, 40 + 1*HELI_SPACE},
        {false, {0.44, 0.68, 0.28}, 40 + 2*HELI_SPACE},
        {true, {0.27, 0.45, 0.77}, 40 + 3*HELI_SPACE}
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
                Rect::draw({0, 0, 100, 37.5}, {0.900, 0.870, 0.900}); // Bottom area

                player.draw();
                for (auto &heli : helis) heli.draw();

                glColor3f(0.000, 0.000, 0.000);
                renderBitmapString(79, 98, (void *) FONT1, "Kalan Hak:");
                renderBitmapString(59, 98, (void *) FONT1, "Puan:");

                glColor3f(1.000, 0.000, 0.000);
                renderBitmapString(98, 98, (void *) FONT1, to_string(player.health()).c_str());
                renderBitmapString(69, 98, (void *) FONT1, to_string(player.score()).c_str());
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
