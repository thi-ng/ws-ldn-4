import java.nio.*;

String basePath = "/Users/toxi/dev/arm/workspace2/ws-ldn-4/assets/";
String imgName = "BlackAngle_64_16";

PImage img = loadImage(basePath + imgName + ".png");
ByteBuffer buf = ByteBuffer.allocate(img.pixels.length*4);
buf.order(ByteOrder.LITTLE_ENDIAN);

for(int i = 0; i< img.pixels.length; i++) {
  buf.putInt(img.pixels[i]);
}

saveBytes(basePath + imgName + ".raw", buf.array());
println("done");
exit();
