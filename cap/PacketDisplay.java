//
// PacketDisplay.java
//
// Andrew Keesler
//
// December 14, 2014
//
// Abstraction of a human-readable packet.

/**
 * An abstraction for a packet display.
 *
 * Add header lines to it with {@link #headerLine} and detail lines to it
 * with {@line #detailLine}. Detail lines are more indented than header
 * lines are.
 *
 * @author Andrew Keesler <ankeesler1@gmail.com>
 * @data December 15, 2014
 *
 */
public class PacketDisplay {

  private StringBuilder builder = new StringBuilder();

  /**
   * Add a header line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void headerLine(String key, String value) {
    kvp(key, value, true);
  }

  /**
   * Add a detail line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void detailLine(String key, int value) {
    kvp(key, String.format("%d", value), false);    
  }

  /**
   * Add a detail line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void detailLine(String key, String value) {
    kvp(key, value, false);    
  }

  private void kvp(String key, String value, boolean header) {
    builder.append(String.format("%s%s: %s\n",
                                 (header ? "" : "  "),
                                 key,
                                 value));
  }
  
  @Override
  public String toString() { return this.builder.toString(); }

}
