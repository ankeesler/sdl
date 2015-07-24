//
// SdlLogSdlListener.java
//
// Andrew Keesler
//
// July 22, 2015
//
// Listener for SDL data.

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import java.util.*;

public class SdlLogSdlListener extends SdlLogBaseListener {

  // The key used for decryption. If this is null, decryption will
  // not be used.
  private final String key;

  /** Create a SDL decoder, optionally supplying a key for decryption.
   *
   * If the key is null, decryption will not be applied. Else, the supplied
   * key will be used for decryption.
   *
   * @param key The key to be used for decryption. If this is null, then
   * no decryption will be used.
   */
  public SdlLogSdlListener(String key) {
    this.key = key;
  }

  private static final int SDL_PHY_AND_MAC_PDU_LEN = 13; 

  private static List<Integer> decrypt(List<Integer> bytes, String key) {
    List<Integer> plaintext = null;

    // Don't apply descryption if key is null.
    if (key == null) return bytes;

    plaintext = new ArrayList<Integer>();

    for (int cryptI = 0, keyI = 0; cryptI < bytes.size(); cryptI ++) {
      int crypt = bytes.get(cryptI);
      int plain = crypt ^ key.charAt(keyI);

      // Stupid java with its all signed types.
      if (plain < 0) plain += 255;

      plaintext.add(plain);

      if (++keyI == key.length()) {
        keyI = 0;
      }
    }

    return plaintext;
  }

  private static int decodeSdlAddress(List<Integer> bytes) {
    int o4 = bytes.remove(0);
    int o3 = bytes.remove(0);
    int o2 = bytes.remove(0);
    int o1 = bytes.remove(0);

    return ((    o4 << 0x18  )
            | (  o3 << 0x10  )
            | (  o2 << 0x08  )
            | (  o1 << 0x00  ));
  }

  private static void decodeSdl(PacketDisplay packet,
                                List<Integer> bytes,
                                String key) {
    // The first byte is the PHY header, i.e., the length of the whole packet.
    int packetLength = bytes.remove(0);
    packet.headerLine("SDL", "PHY");
    packet.detailLine("Length", packetLength);

    // MAC.
    packet.headerLine("SDL", "MAC (key = " + key + ")");

    // Two bytes for the frame control.
    int high = bytes.remove(0);
    int low = bytes.remove(0);
    int frameControl = ((high << 0x08) | low);
    packet.detailLine("Frame Control", String.format("0x%04X", frameControl));
    packet.subdetailLine("Type", "Data (0)");

    // Two bytes for the sequence number.
    high = bytes.remove(0);
    low = bytes.remove(0);
    int sequence = ((high << 0x08) | low);
    packet.detailLine("Sequence", String.format("0x%04X", sequence));

    // Four bytes for the source and destination address.
    int source      = decodeSdlAddress(bytes);
    int destination = decodeSdlAddress(bytes);
    packet.detailLine("Source Address",
                      String.format("0x%08X", source));
    packet.detailLine("Destination Address",
                      String.format("0x%08X", destination));

    // Data.
    DecodeUtil.decodeBytes(packet,
                           decrypt(bytes, key),
                           packetLength - SDL_PHY_AND_MAC_PDU_LEN);
  }

  @Override
  public void enterPacket(SdlLogParser.PacketContext ctx) {
    PacketDisplay packet = new PacketDisplay();
    List<Integer> bytes = DecodeUtil.bytes(ctx.DATA().getText());

    packet.titleLine("Time", "" + ctx.TIMESTAMP().getText() + " s");
    packet.titleLine("Direction", ctx.DIRECTION().getText());

    decodeSdl(packet, bytes, key);

    System.out.println(packet);
  }
}

