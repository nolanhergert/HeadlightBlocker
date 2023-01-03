/*

Made with textik
                                                                                                           
                                   d1                                                                     
      (0,0)                   xInt                                           (xMax,0)                     
            -------------------|----------------------------------------------/-                          
             \                 |                                            /-                            
              \                |                                         /--                              
               \               |                                       /-                                 
                \              |                                    /--                                   
                 \             |                                  /-                                      
                  \            |                               /--                                        
                   \           | yMax                        /-                                           
                    \          |                          /--                                             
                     \         |                       /--                                                
                      \        |                     /-                                                   
                 d2    \       |                  /--      d3                                             
                        \      |                /-                                                        
                         \     |             /--                                                          
                          \    |           /-                                                             
                           \   |        /--                                                               
                            \  |      /-                                                                  
                             \ |   /--                                                                    
                              \| /-                                                                       
                               --                                                                         
                           (xInt,yMax)                                                            

*/

// Constant at run-time, but not compile time. Need to read settings from flash
double d1 = 3.3; // top left to top right
double d2 = 1.4; // top left to bottom
double d3 = 2.9; // bottom to top right

constexpr double computeXInt(double d1, double d2, double d3) {
  return (sq(d1) + sq(d2) - sq(d3)) / (2 * d1);
}

constexpr double computeYMax(double d2, double xInt) {
  return sqrt(sq(d2) - sq(xInt));
}

static_assert(abs(computeXInt(3.3, 1.4, 2.9) - 0.67) < 0.05);
static_assert(abs(computeYMax(1.4, 0.67) - 1.25) < 0.05);


// Use AccelStepper or FastAccelStepper instead? Seems easier to manage and it does math too...and doesn't use interrupts, just loop().

class MicroStepperMotor {
  const int TicksPerMm;
  const double shaftX;
  const double shaftY;
  // Motor's idea of the light blocker's X and Y
  double currentX = 0;
  double currentY = 0;
  double targetX = 0;
  double targetY = 0;

  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

}
