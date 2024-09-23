#include <omp.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace cv;
using namespace std;

// Simulation window dimensions
int width = 1500;
int height = 800;

class Ball;
class QuadTree;

// Global vector to store all balls
vector<Ball> balls;

// Function declarations
void display(Mat& img);
void createMap(const vector<Ball>& balls, Mat& img);
void makeDecision(vector<Ball>& balls, QuadTree& quadTree);
void createBalls(int numberOfBalls, int diameter, const std::string& type);
void moveBalls(vector<Ball>& balls);
void checkCollisions(vector<Ball>& balls, QuadTree& quadTree);

//******************************************************Ball class***************************************************************//
class Ball {
public:
    int diameter; // Diameter of the ball
    int position[2]; // Position of the ball (x, y)
    std::string type; // Type of the ball (normal, aggressive, etc.)
    int direction; // Direction of the ball's movement (0-7, representing 8 possible directions)
    float turningParameter; // Probability of changing direction
    int speed; // Speed of the ball

    // Default constructor
    Ball()
        : diameter(50), type("normal"), direction(rand() % 8), turningParameter(0.01f), speed(1) {
        position[0] = 100;
        position[1] = 100;
        balls.push_back(*this);
    }

    // Parameterized constructor
    Ball(int diam, int posX, int posY, const std::string& ballType, float turnParam = 0.01f)
        : diameter(diam), type(ballType), direction(rand() % 8), turningParameter(turnParam) {
        position[0] = posX;
        position[1] = posY;
        speed = (type == "aggressive") ? 2 : 1;
        balls.push_back(*this);
    }

    // Moves the ball according to its direction and speed
    void move() {
        // Change direction if the ball hits the window boundaries
        if (position[0] >= width - diameter / 2 || position[1] <= diameter / 2 ||
            position[0] <= diameter / 2 || position[1] >= height - diameter / 2) {
            direction = (direction + 4) % 8;
        }
        switch (direction) {
        case 0: position[1] -= speed; break;
        case 1: position[0] += speed; position[1] -= speed; break;
        case 2: position[0] += speed; break;
        case 3: position[0] += speed; position[1] += speed; break;
        case 4: position[1] += speed; break;
        case 5: position[0] -= speed; position[1] += speed; break;
        case 6: position[0] -= speed; break;
        case 7: position[0] -= speed; position[1] -= speed; break;
        }

        // Ensure the ball stays within bounds
        if (position[0] < diameter / 2) position[0] = diameter / 2;
        if (position[1] < diameter / 2) position[1] = diameter / 2;
        if (position[0] > width - diameter / 2) position[0] = width - diameter / 2;
        if (position[1] > height - diameter / 2) position[1] = height - diameter / 2;
    }
};

//******************************************************quadTree class***************************************************************//

// Represents a rectangular area in the simulation
class Rectangle {
public:
    int x, y, width, height;

    // Constructor
    Rectangle(int x, int y, int width, int height)
        : x(x), y(y), width(width), height(height) {}

    // Check if a ball is within the rectangle
    bool containsBall(const Ball& ball) const {
        int xb = ball.position[0];
        int yb = ball.position[1];
        return (xb > x && xb < x + width && yb > y && yb < y + height);
    }

    // Check if this rectangle intersects another rectangle
    bool intersects(const Rectangle& range) const {
        return !(range.x > x + width || range.x + range.width < x ||
            range.y > y + height || range.y + range.height < y);
    }
};

// Represents a QuadTree node used for spatial partitioning
class QuadTree {
private:
    static const int CAPACITY = 4; // Max number of balls a QuadTree node can hold before subdividing
    Rectangle boundary; // Boundary of this QuadTree node
    vector<Ball*> balls; // Balls contained in this node
    bool divided; // Indicates if this node has been subdivided

    // Child QuadTree nodes
    QuadTree* northeast;
    QuadTree* northwest;
    QuadTree* southeast;
    QuadTree* southwest;

public:
    // Constructor
    QuadTree(const Rectangle& boundary)
        : boundary(boundary), divided(false), northeast(nullptr), northwest(nullptr), southeast(nullptr), southwest(nullptr) {}

    // Inserts a ball into the QuadTree
    bool insert(Ball* ball) {
        if (!boundary.containsBall(*ball)) return false;
        if (balls.size() < CAPACITY) {
            balls.push_back(ball);
            return true;
        }
        if (!divided) subdivide();
        if (northeast->insert(ball)) return true;
        if (northwest->insert(ball)) return true;
        if (southeast->insert(ball)) return true;
        if (southwest->insert(ball)) return true;
        return false;
    }

    // Subdivides the QuadTree node into four child nodes
    void subdivide() {
        int x = boundary.x;
        int y = boundary.y;
        int w = boundary.width / 2;
        int h = boundary.height / 2;

        northeast = new QuadTree(Rectangle(x + w, y, w, h));
        northwest = new QuadTree(Rectangle(x, y, w, h));
        southeast = new QuadTree(Rectangle(x + w, y + h, w, h));
        southwest = new QuadTree(Rectangle(x, y + h, w, h));

        divided = true;
    }

    // Queries the QuadTree to find all balls within a given range
    void query(const Rectangle& range, vector<Ball*>& found) const {
        if (!boundary.intersects(range)) return;
        for (const Ball* ball : balls) {
            if (range.containsBall(*ball)) found.push_back(const_cast<Ball*>(ball));
        }
        if (divided) {
            northeast->query(range, found);
            northwest->query(range, found);
            southeast->query(range, found);
            southwest->query(range, found);
        }
    }
};

// Insert each ball into the QuadTree
void insertBallsIntoQuadTree(vector<Ball>& balls, QuadTree& quadTree) {
    for (int i = 0; i < balls.size(); ++i) {
        quadTree.insert(&balls[i]);
    }
}

//******************************************************main program***************************************************************//

// Main program
int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed the random number generator

    Mat Map(height, width, CV_8UC3, Scalar(255, 255, 255)); // Create an empty map with a white background

    createBalls(200000, 1, "aggressive"); // Create 200,000 aggressive balls

    // Define the window as a rectangle and create the QuadTree
    Rectangle window(0, 0, width, height);
    QuadTree quadTree(window);

    while (true) {
        makeDecision(balls, quadTree); // Update the state of each ball
        createMap(balls, Map); // Draw the balls on the map
        display(Map); // Display the map
        if (waitKey(10) >= 0) break; // Exit if a key is pressed
    }

    return 0;
}

//******************************************************desicion making functions***************************************************************//

// Update the state of each ball
void makeDecision(vector<Ball>& balls, QuadTree& quadTree) {

    double time1 = omp_get_wtime();//get timestamp1

    quadTree = QuadTree(Rectangle(0, 0, width, height)); // Reinitialize the QuadTree for the current frame

    double time2 = omp_get_wtime();//get timestamp2

    moveBalls(balls); // Move each ball

    double time3 = omp_get_wtime();//get timestamp3

    insertBallsIntoQuadTree(balls, quadTree); // Insert balls into the QuadTree

    double time4 = omp_get_wtime();//get timestamp4

    checkCollisions(balls, quadTree); // Check for collisions

    double time5 = omp_get_wtime();//get timestamp4

    double time = time5 - time4;
    cout << time * 1000 << endl;  // Calculate time difference
    //conclution timestamp 4 to 5 takes 75% procesing time
    //checkCollisions function must be improved
}

// Create a specified number of balls with given diameter and type
void createBalls(int numberOfBalls, int diameter, const std::string& type) {
    for (int i = 0; i < numberOfBalls; ++i) {
        int posX = rand() % width;
        int posY = rand() % height;
        Ball newBall(diameter, posX, posY, type, 0.02);
    }
}

// Move each ball according to its current state
void moveBalls(vector<Ball>& balls) {
    for (int i = 0; i < balls.size(); ++i) {
        float x = static_cast<float>(rand()) / RAND_MAX;    //has a problem with parallel threads
        if (x < balls[i].turningParameter) {
            balls[i].direction = (balls[i].direction + 1) % 8;
        }
        else if (x > 1 - balls[i].turningParameter) {
            balls[i].direction = (balls[i].direction + 7) % 8;
        }
        if (balls[i].type != "dead") {
            balls[i].move();
        }
    }
}

// Check for collisions between balls
void checkCollisions(vector<Ball>& balls, QuadTree& quadTree) {
    int numThreads = omp_get_max_threads();
    vector<bool> ballIsDead(balls.size(), false);

#pragma omp parallel
    {
        vector<Ball*> found;
        int threadID = omp_get_thread_num();

#pragma omp for schedule(dynamic)
        for (int i = 0; i < balls.size(); ++i) {
            if (balls[i].type != "dead") {
                found.clear();
                Rectangle range(balls[i].position[0] - balls[i].diameter, balls[i].position[1] - balls[i].diameter, balls[i].diameter * 2, balls[i].diameter * 2);
                quadTree.query(range, found);  // Use the QuadTree to find nearby balls

                for (auto& otherBall : found) {
                    if (otherBall != &balls[i] && otherBall->type != "dead") {
                        int deltaX = balls[i].position[0] - otherBall->position[0];
                        int deltaY = balls[i].position[1] - otherBall->position[1];
                        int distanceSquared = deltaX * deltaX + deltaY * deltaY;
                        int combinedRadius = (balls[i].diameter + otherBall->diameter) / 2;
                        if (distanceSquared <= combinedRadius * combinedRadius) {
                            ballIsDead[i] = true;
                            ballIsDead[otherBall - &balls[0]] = true;
                        }
                    }
                }
            }
        }
    }

#pragma omp parallel for schedule(static,1)
    for (int i = 0; i < balls.size(); ++i) {
        if (ballIsDead[i]) {
            balls[i].type = "dead";
        }
    }
}





//******************************************************openCV***************************************************************//

// Displays the current state of the map using OpenCV
void display(Mat& img) {
    imshow("Simulation", img); // Display the image in a window named "Simulation"
}

// Draws all balls on the map
void createMap(const vector<Ball>& balls, Mat& img) {
    img.setTo(Scalar(255, 255, 255)); // Clear the map with a white background
    for (const auto& ball : balls) {
        if (ball.type != "dead") {
            circle(img, Point(ball.position[0], ball.position[1]), ball.diameter / 2, Scalar(0, 0, 0), -1); // Draw each ball as a black circle
        }
    }
}

