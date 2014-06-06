/*#include <Fat16.h>

SdCard card;
Fat16 file;

void saveInit() {
  card.init();
  Fat16::init(&card);
  
  char name[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write only
    if (file.open(name, O_CREAT | O_EXCL | O_WRITE))
      break;
  }
}

void saveTask() {
  if (!file.isOpen()) return;
  
  file.println(10);

  //if (file.writeError) error("write data");
  
  file.sync();
}*/
