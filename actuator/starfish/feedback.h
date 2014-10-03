/*
 * for computing closed loop feedback
 */
typedef enum {FB_DISABLED, FB_OPEN_LOOP, FB_POSITION, FB_VELOCITY, FB_FORCE, FB_CURRENT} FeedbackType;
void updateCoeffs(float pGain, float iGain, float dGain, FeedbackType fbt);
void calcFeedback(float setpoint, float position, float current, float torque);
void feedbackInit(void);
FeedbackType getFeedbackType();
float getProportionalGain();
float getIntegralGain();
float getDerivativeGain();
float getVelocity();
#define PI 3.1415926535897932384626433832795028841f

