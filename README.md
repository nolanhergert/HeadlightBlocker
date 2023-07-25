# HeadlightBlocker
## Features
  * Will block bright lights anywhere in your vision, including side view mirrors!
  * Impact safe? with double sided 4 mil security film. Withstands a hard impact without glass shards (I think...): https://youtu.be/FGfVA1lMs6I?t=80

See the wiki page: https://github.com/nolanhergert/HeadlightBlocker/wiki

POC OpenCV code (for motor-driver solutionj, potentially obsolete) is here: https://github.com/nolanhergert/lib/blob/master/python/examples/headlight_blocker.py

Can leverage CH32V003fun to read from camera using DMA? Use this example https://github.com/cnlohr/ch32v003fun/blob/master/examples/dma_gpio/dma_gpio.c but change this line to be INDR? `DMA1_Channel2->PADDR = (uint32_t)&GPIOC->OUTDR;`
