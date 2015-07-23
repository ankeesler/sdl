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

  // Returns next header.
  private Integer decodeIp(PacketDisplay packet, List<Integer> bytes) {
    packet.headerLine("Ipv6", "Data"); // TODO: put something for the value here

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
      return Ipv6.nextHeader("No Next Header");

    // Payload length.
    int high = bytes.remove(0);
    int low  = bytes.remove(0);
    int payloadLength = low | (high << 8);
    packet.detailLine("Payload length", payloadLength);

    // Next header.
    int nextHeader = bytes.remove(0);
    String name = Ipv6.nextHeader(nextHeader);
    packet.detailLine("Next header",
                      (""
                       + nextHeader
                       + " ("
                       + (name == null ? "Unknown" : name)
                       + ")"));

    // Hop limit.
    packet.detailLine("Hop limit", bytes.remove(0));

    try {
      // Source address.
      packet.detailLine("Source", ipv6Address(bytes));

      // Destination address.
      packet.detailLine("Destination", ipv6Address(bytes));
    } catch (Exception e) {}

    return nextHeader;
  }

  private void decodeIcmpv6(PacketDisplay packet, List<Integer> bytes) {
    int type = bytes.remove(0);
    String typeName = Ipv6.icmpv6Type(type);
    int code = bytes.remove(0);

    packet.headerLine("ICMPv6", (typeName == null ? "" : typeName));

    // Type.
    packet.detailLine("Type",
                      String.format("%d (%s)",
                                    type,
                                    (typeName == null ? "Unknown" : typeName)));

    // Code.
    packet.detailLine("Code",
                      String.format("%d",
                                    code));

    // Checksum.
    int checksum = ((bytes.remove(0) & 0x000000FF) << 8);
    checksum |= (bytes.remove(0) & 0x000000FF);
    packet.detailLine("Checksum", String.format("0x%04X", checksum));

    // Data.
    DecodeUtil.decodeBytes(packet, bytes, bytes.size());
  }
  
  @Override
  public void enterPacket(SdlLogParser.PacketContext ctx) {
    PacketDisplay packet = new PacketDisplay();
    List<Integer> bytes = DecodeUtil.bytes(ctx.DATA().getText());

    packet.titleLine("Time", "" + ctx.TIMESTAMP().getText() + " s");
    packet.titleLine("Direction", ctx.DIRECTION().getText());

    Integer nextHeader = decodeIp(packet, bytes);
    if (nextHeader != null) {
      if (nextHeader == 58)
        decodeIcmpv6(packet, bytes);
    }

    System.out.println(packet);
  }

}
