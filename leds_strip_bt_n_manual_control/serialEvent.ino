void serialEvent(){
  for (int i=0; i<COMMAND_LENGTH; i++) {
    command[i] = '\0';
  }
  sIt = 0;
  if (Serial.available()) {
    Serial.print("<< Sent: ");
    while (Serial.available()) {
      c = Serial.read();
      command[sIt] = c;
      sIt++;
      BT.write(c);
    }
  }
  if (BT.available()) {
    Serial.print(">> Read: ");
    while (BT.available()) {
      c = BT.read();
      command[sIt] = c;
      sIt++;
    }
  }
  analizeString(command);
}

void analizeString(char command[COMMAND_LENGTH]){
  Serial.println(command);
  volatile int H = 0;
  volatile int S = 0;
  volatile int V = 0;
  volatile int tokens_counter = 0;
  volatile int strip_indx = 0;
  char *token;
  switch (command[0]) {
    case '0': // Reference signal
              Serial.print("<< ");
              for (int i=0; i<sizeof(data); i++) {
                BT.write(data[i]);
                Serial.print((char) data[i]);
              }
              Serial.println();
      break;
    case 'X': // Power off.
              all_off();
              sendOK();
      break;
    case 'B': 
              token = strtok(command, delimiter);
              tokens_counter = 0;
              while(token != NULL) {
                tokens_counter += 1;
                token = strtok(NULL, delimiter);
                
                if (tokens_counter == 1) {
                  strip_indx = bound(atoi(token), 0, N);
                }
                else if (tokens_counter == 2) {
                  brght = bound((int) 100 * ((float) (atoi(token)-5) / 70), 0, 100);
                  brghts[strip_indx] = brght;
                }
              }
              sendOK();
      break;
    case 'F': 
              token = strtok(command, delimiter); // The first token == command[0], already processed.
              token = strtok(NULL, delimiter);
              freq = bound(minFreq + (int) ((float) atoi(token)/100.0 * (maxFreq-minFreq)), (int) minFreq, (int) maxFreq);
              freq = bound((int) (15 * exp(0.05*((float) atoi(token)-15))), (int) minFreq, (int) maxFreq);;
              sendOK();
      break;
    case 'N': 
              token = strtok(command, delimiter);
              tokens_counter = 0;
              while(token != NULL) {
                tokens_counter += 1;
                token = strtok(NULL, delimiter);
                
                if (tokens_counter == 1) {
                  strip_indx = bound(atoi(token), 0, N);
                }
                else if (tokens_counter == 2) {
                  num = bound(atoi(token), 0, 100);
                  nums[strip_indx] = (int) ((float) num / 100 * og_nums[strip_indx]);
                }
              }
              sendOK();
      break;
    case 'P': // Reference signal
              // Find the first delimited word.
              token = strtok(command, delimiter);
              
              tokens_counter = 0;
              // Find the following delimited words.
              while(token != NULL) {
                Serial.println(token);
                tokens_counter += 1;
                token = strtok(NULL, delimiter);

                if (tokens_counter == 1) {
                  strip_indx = bound(atoi(token), 0, N);
                }
                else if (tokens_counter == 2) {
                  palettes[strip_indx] = bound(atoi(token), 0, 10); // The length of the palettes array should be checked here.
                }
                // else if (palettes[strip_indx] > 
                //   tokens_counter == 3) {
                //   R = atoi(token);
                //   Rs[strip_indx] = bound(R, 0, 255);
                // }
                // else if (tokens_counter == 4) {
                //   G = atoi(token);
                //   Gs[strip_indx] = bound(G, 0, 255);
                // }
                // else if (tokens_counter == 5) {
                //   B = atoi(token);
                //   Bs[strip_indx] = bound(B, 0, 255);
                // }
              }
              sendOK();
      break;
    default:
              Serial.print("ERROR: Command: '");
              Serial.print(command);
              Serial.println("' is not valid."); 
              sendOK();
      break;
  }

  for (int i=0; i<N; i++) {
    Serial.print(i);
    Serial.print(" = N: ");
    Serial.print(nums[i]);
    Serial.print("; B: ");
    Serial.print(brghts[i]);
    Serial.print("; P: ");
    Serial.println(palettes[i]);
  }
  Serial.print("F: ");
  Serial.print(freq);
  Serial.println(" %;");
  delay(100);
}

int bound(int var, int min, int max) {
  if (var < min) {
    var = min;
  }
  if (var > max) {
    var = max;
  }
  return var;
}

void sendOK() {
  for (int i=0; i<sizeof(ok); i++) {
    BT.write(ok[i]);
    Serial.print((char) ok[i]);
  }
  Serial.println();
}
