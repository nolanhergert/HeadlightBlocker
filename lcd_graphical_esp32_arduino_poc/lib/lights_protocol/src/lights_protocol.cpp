

void ResetString() {
  tempStrLength = 0;
  // This doesn't seem to be working...
  memset(tempStr, '\0', tempStrLengthMax);
}

void ProcessString(char * str, uint16_t length) {
  numLights = 0;

  int i = 0;
  int currentXIndex = -1;
  int currentYIndex = -1;
  // TODO: Now we have the whole string. Need to parse.
  // Reimplementing strtok as I need a little more information
  for (int i = 0; i < length; i++) {
    if (currentXIndex == -1) {
      currentXIndex = i;
      continue;
    }
    if (str[i] != ' ' && str[i] != '\n') {
      continue;
    }

    if (currentYIndex == -1) {
      currentYIndex = i;
      continue;
    }

    // Now we have reached the start of the next light
    lights[numLights].x1 = atoi(&str[currentXIndex]);
    lights[numLights].y1 = atoi(&str[currentYIndex]);
    lights[numLights].radius = 10;
    Serial.printf("Light: %d %d\n", lights[numLights].x1, lights[numLights].y1 );

    // TODO: Change this back to CameraToLCD
    //lights[numLights].y1 = lcd_height - lights[numLights].y1;
    CameraToLCD(&lights[numLights].x1, &lights[numLights].y1);
    numLights++;

    currentXIndex = -1;
    currentYIndex = -1;
  }
}




  uint16_t numBytesToRead = Serial_UART.available();

  // Want at least one total sequence
  if (numBytesToRead < tempStrLengthMax / 2) {
    return;
  }

  // If there is too much, then drop the earlier bytes
  // TODO: Don't need to actually read these bytes
  while (numBytesToRead >= tempStrLengthMax) {
    Serial_UART.readBytes(tempStr, tempStrLengthMax);
    numBytesToRead -= tempStrLengthMax;
  }

  Serial_UART.readBytes(tempStr, numBytesToRead);

  Serial.println(tempStr);

  // Find the last complete sequence
  for (i = numBytesToRead-1; i >= 0; i--) {
    if (tempStr[i] == '\n') {
      if (indexLastNewline == -1) {
        indexLastNewline = i;
      } else if (index2ndToLastNewline == -1) {
        index2ndToLastNewline = i;
        break;
      }
    }
  }

  assert(indexLastNewline != -1);
  assert(index2ndToLastNewline != -1);

  Serial.printf("Processing! %i, %i: %s", index2ndToLastNewline, indexLastNewline, &tempStr[index2ndToLastNewline+1]);

  // I really should be reading into a circular buffer and wait until the
  // next newline received. Shouldn't need asserts either.



  ProcessString(&tempStr[index2ndToLastNewline+1], indexLastNewline - index2ndToLastNewline);