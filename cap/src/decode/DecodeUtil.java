//
// DecoderUtil.java
//
// Andrew Keesler
//
// July 23, 2015
//
// Decoder utilities.

import java.util.*;

public class DecodeUtil {

  public static List<Integer> bytes(String stuff) {
    // Take off first and last '['.
    int stringLength = stuff.length();
    String[] data = stuff.substring(1, stringLength-1).split(",");
    List<Integer> bytes = new ArrayList<Integer>(data.length);

    // Add the bytes.
    for (int i = 0; i < data.length; i++)
      bytes.add(Integer.parseInt(data[i].trim().substring(2), 16));

    return bytes;
  }

  // Organized by word.
  public static void decodeBytes(PacketDisplay packet,
                                 List<Integer> bytes,
                                 int length) {
    StringBuilder builder = new StringBuilder();
    int i = 0;
    for (; i < length; i ++) {
      if (i % 4 == 3) {
        builder.append(String.format("0x%02X", bytes.remove(0)));
        packet.detailLine("Data", builder.toString());
        builder.delete(0, builder.length());
      } else {
        builder.append(String.format("0x%02X, ", bytes.remove(0)));
      }
    }
    if (i % 4 != 0) {
      packet.detailLine("Data", builder.toString());
    }
  }
}
