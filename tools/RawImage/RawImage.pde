import java.nio.*;

String basePath = "/Users/toxi/dev/arm/workspace2/ws-ldn-4/assets/";
//String imgName = "BlackAngle_64_16";
String imgName = "DustLED2_48_2";
boolean isARGB = false;
color bgColor = #59626c;

void setup() {
  PImage img = loadImage(basePath + imgName + ".png");
  size(img.width, img.height);
  background(bgColor);
  image(img, 0, 0);
  loadPixels();
  ByteBuffer buf = ByteBuffer.allocate(img.pixels.length * (isARGB ? 4 : 3));
  buf.order(ByteOrder.LITTLE_ENDIAN);

  if (isARGB) {
    for (int i = 0; i< img.pixels.length; i++) {
      buf.putInt(img.pixels[i]);
    }
  } 
  else {
    // if not ARGB, read composited pixels from window's pixel buffer
    for (int i = 0; i< img.pixels.length; i++) {
      int c = pixels[i];
      buf.put((byte)(c & 0xff));
      buf.put((byte)((c >> 8) & 0xff));
      buf.put((byte)((c >> 16) & 0xff));
    }
  }
  saveBytes(basePath + imgName + ".raw", buf.array());
  println("done");
  //exit();
}

