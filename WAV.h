#include <WAV.h>
#include <SD.h>
#include <SPI.h>

const int SD_CS_PIN = D8;

const char* filePath = "/audio.wav";
File audioFile;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Ошибка инициализации SD-карты!");
    return;
  }

  audioFile = SD.open(filePath, FILE_WRITE);
  if (!audioFile) {
    Serial.println("Не удалось создать аудиофайл!");
    return;
  }

  // Создание заголовка WAV файла
  WAVHeader wavHeader;
  strncpy(wavHeader.chunkID, "RIFF", 4);
  strncpy(wavHeader.format, "WAVE", 4);
  strncpy(wavHeader.subchunk1ID, "fmt ", 4);
  wavHeader.subchunk1Size = 16;
  wavHeader.audioFormat = 1;
  wavHeader.numChannels = 1;
  wavHeader.sampleRate = 16000;
  wavHeader.bitsPerSample = 16;
  wavHeader.byteRate = wavHeader.sampleRate * wavHeader.numChannels * (wavHeader.bitsPerSample / 8);
  wavHeader.blockAlign = wavHeader.numChannels * (wavHeader.bitsPerSample / 8);
  strncpy(wavHeader.subchunk2ID, "data", 4);
  wavHeader.subchunk2Size = 0; // Обновится после записи аудиоданных

  audioFile.write((const char*)&wavHeader, sizeof(wavHeader));

  // Запись аудиоданных
  // TODO: добавьте код для записи аудиоданных

  // Обновление размера блока данных в заголовке WAV файла
  long audioDataSize = audioFile.size() - sizeof(wavHeader);
  audioFile.seek(sizeof(wavHeader) - sizeof(wavHeader.subchunk2Size));
  audioFile.write((const char*)&audioDataSize, sizeof(audioDataSize));

  audioFile.close();
}

void loop() {
}
