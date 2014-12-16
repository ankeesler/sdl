//
// SdlLogIpv6Listener.java
//
// Andrew Keesler
//
// December 13, 2014
//
// Translate hex bytes in an Ipv6 packet to something that makes sense.

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import java.util.*;

public class SdlLogIpv6Listener extends SdlLogBaseListener {

  public static final Map<Integer, String> NEXT_HEADER_MAP
    = new HashMap<Integer, String>();

  static {
    NEXT_HEADER_MAP.put(6, "TCP");
    NEXT_HEADER_MAP.put(17, "UDP");
    NEXT_HEADER_MAP.put(1, "ICMP");
    NEXT_HEADER_MAP.put(2, "IGMP");
    NEXT_HEADER_MAP.put(58, "ICMPv6");

    NEXT_HEADER_MAP.put(50, "ESP Header");
    NEXT_HEADER_MAP.put(51, "Authentication Header");

    NEXT_HEADER_MAP.put(0, "Hop-by-Hop Options Header");
    NEXT_HEADER_MAP.put(43, "Routing Header");
    NEXT_HEADER_MAP.put(44, "Fragment Header");
    NEXT_HEADER_MAP.put(59, "No Next Header");
    NEXT_HEADER_MAP.put(60, "Destination Options Header");

    NEXT_HEADER_MAP.put(41, "Ipv6 Encapsulation");
  };

  private static List<Integer> bytes(String stuff) {
    // Take off first and last '['.
    int stringLength = stuff.length();
    String[] data = stuff.substring(1, stringLength-1).split(",");
    List<Integer> bytes = new ArrayList<Integer>(data.length);

    // Add the bytes.
    for (int i = 0; i < data.length; i++)
      bytes.add(Integer.parseInt(data[i].trim().substring(2), 16));

    return bytes;
  }

  private static String ipv6Address(List<Integer> bytes) throws Exception {
    StringBuilder address = new StringBuilder();
    boolean skippingZeros = false;
    boolean skippedZeros = false;
    int value;

    // Pull off in two byte increments, so 7 times and then one more.
    for (int i = 0; i < 7; i ++) {
      value  = bytes.remove(0) << 8;
      value |= bytes.remove(0) << 0;
      if (value == 0 && !skippedZeros) {
        if (!skippingZeros)
          address.append((i == 0 ? "::" : ":"));
        skippingZeros = true;
      } else {
        if (skippingZeros)
          skippedZeros = true;
        address.append(String.format("%04X:", value));
      }
    }

    value  = bytes.remove(0) << 8;
    value |= bytes.remove(0) << 0;
    if (value == 0) {
      if (!skippingZeros && !skippedZeros)
        address.append(":");
    } else {
      address.append(String.format("%04X", value));
    }

    return address.toString();
  }

  @Override
  public void enterPacket(SdlLogParser.PacketContext ctx) {
    PacketDisplay packet = new PacketDisplay();
    List<Integer> bytes = bytes(ctx.DATA().getText());

    packet.headerLine("Time", "" + ctx.TIMESTAMP().getText() + " s");
    packet.headerLine("Direction", ctx.DIRECTION().getText());

    // Big-endian.

    int first  = bytes.remove(0);
    int second = bytes.remove(1);
    // Version.
    packet.detailLine("Version", (first & 0xF0) >> 4);
    // Traffic class.
    int trafficClass = (first & 0x0F) | (second & 0xF0);
    packet.detailLine("Traffic class", trafficClass);

    // Flow label.
    int third  = bytes.remove(0);
    int fourth = bytes.remove(0);
    int flowLabel = (second & 0xF0 << 12) | (third << 4) | (fourth);
    packet.detailLine("Flow label", flowLabel);

    if (bytes.isEmpty())
      return;

    // Payload length.
    int high = bytes.remove(0);
    int low  = bytes.remove(0);
    int payloadLength = low | (high << 8);
    packet.detailLine("Payload length", payloadLength);

    // Next header.
    int nextHeader = bytes.remove(0);
    String name = NEXT_HEADER_MAP.get(nextHeader);
    packet.detailLine("Next header",
                      "" + nextHeader + (name == null ? "Unknown" : name));

    // Hop limit.
    packet.detailLine("Hop limit", bytes.remove(0));

    try {
      // Source address.
      packet.detailLine("Source", ipv6Address(bytes));

      // Destination address.
      packet.detailLine("Destination", ipv6Address(bytes));
    } catch (Exception e) {}

    System.out.println(packet);
  }

}
