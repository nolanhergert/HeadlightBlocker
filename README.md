# HeadlightBlocker

## CV Algorithm
Don't really need initial training, since it's kind of hard to set up. Just train/learn on the fly.

  * Acquire face on spacebar. Then use LK trackers to 1) know where the face currently is and its size (for better prediction next time) and 2) TrackWarpCrop new face back to original face to do shadow detection.
  * Shadow detection. Do brightness/color comparison to original acquired face (without blocking).
  * Motion:
    * If no trained model, move steppers to center of windshield if possible. Detect center of blob(s). It should be centered on the eyes. Move steppers appropriately. Take samples to create model along the way. Encourage moving face around to make motion faster in the future.
    * With trained model, just move there! And adjust if needed.
    
## Stepper motion
### Prototype
Just do one line in a diagonal.

### Gen 1
 * Assume rectangular positioning of motors. <strikethrough>I think horizontal and vertical distances do not need to be the same from car to car.</strikethrough> Need to know exact positioning, actually. If you want to move a little in x and your x position of a stepper is off, then the amount of r you need is different depending on your x position.
#### Getting exact position
  * Automatically:
    * To get exact position, I'm thinking of disabling/lowering power to all steppers except one under test. And pull on test stepper until stall/skipped steps/higher current draw. That tells you distance from wherever you were. Then do the same for other motors. Maybe with some diagonal stuff thrown in. Need to have less power to some steppers and be able to detect stall. I can do that, right?
      * Detect current draw is pretty simple. Yeah, but it's not useful. We really need to detect skipped steps.
      * Multi channel digital potentiometer to limit current? Probably can't use PWM right? I forget what A4988 uses, I think it's %VCC though.
  * Manually:
    * Just measure with measuring tape each side and diagonals. Then need to enter using wifi and server. Annoying for most end-users, but not that bad for first prototypes. It's a one-time thing.
 
 * r = sqrt(dx^2 + dy^2). 
 
 
## Blocker 
   * Have removable blocker? That way you don't see lines in daytime. Ehh, I don't think this is that helfpul.
   * Needs to eventually be adjustable to any angle of windshield, or the blocking should be wide enough to handle most angles of windshields.
   
## String
  * Should be strong enough to resist breaking if accidentally grabbed? It's currently quite annoying to restring, especially for end user.
  * Have end caps? Or just tie knot onto center thing.
### Spool
  * I think I can still use a hole through center. UV cure glue in center along with gear. Trim on other side.