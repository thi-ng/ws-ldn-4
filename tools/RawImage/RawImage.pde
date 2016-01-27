import java.nio.*;

String basePath = "/Users/toxi/dev/arm/workspace2/ws-ldn-4/assets/";
//String imgName = "BlackAngle_64_16";
String imgName = "DustLED2_48_2";

int colorMode = 2;
color bgColor = #59626c;

int[] strides = new int[] { 
  4, 3, 2, 2
};

void setup() {
  PImage img = loadImage(basePath + imgName + ".png");
  size(img.width, img.height);
  background(bgColor);
  image(img, 0, 0);
  loadPixels();
  ByteBuffer buf = ByteBuffer.allocate(img.pixels.length * strides[colorMode]);
  buf.order(ByteOrder.LITTLE_ENDIAN);

  switch(colorMode) {
  case 0: // ARGB8888
    for (int i = 0; i< img.pixels.length; i++) {
      buf.putInt(img.pixels[i]);
    }
    break;
  case 1: // RGB888
    for (int i = 0; i< img.pixels.length; i++) {
      int c = pixels[i];
      buf.put((byte)(c & 0xff));
      buf.put((byte)((c >> 8) & 0xff));
      buf.put((byte)((c >> 16) & 0xff));
    }
    break;
  case 2: // ARGB4444
    for (int i = 0; i< img.pixels.length; i++) {
      int c = img.pixels[i];
      int x = ((c >>> 28) << 12) | (((c >> 20) & 0xf) << 8) | (((c >> 12) & 0xf) << 4) | ((c >> 4) & 0xf);
      buf.putShort((short)x);
    }
    break;
  default:
    println("unsupported");
    exit();
  }
  saveBytes(basePath + imgName + "_" + (strides[colorMode] * 8) +".raw", buf.array());
  println("done");
  //exit();
}

