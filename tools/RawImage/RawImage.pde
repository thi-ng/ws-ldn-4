import java.nio.*;

PImage img = loadImage("/Users/toxi/dev/arm/workspace2/ws-ldn-4/assets/tom-robot.png");
ByteBuffer buf = ByteBuffer.allocate(img.pixels.length*4);
buf.order(ByteOrder.LITTLE_ENDIAN);
for(int i = 0; i< img.pixels.length; i++) {
  buf.putInt(img.pixels[i]);
}
saveBytes("/Users/toxi/dev/arm/workspace2/ws-ldn-4/assets/tom-robot.raw", buf.array());

