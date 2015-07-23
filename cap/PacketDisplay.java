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
 * You can add lines to the display using this <code>*line(String, String)</code>
 * APIs. They are formatted as follows.
 *
 * Title Line Key : Title Line Value
 *   Header Line Key : Header Line Value
 *     Detail Line Key : Detail Line Value
 *     Detail Line Key : Detail Line Value
 *       Sub-detail Line Key : Sub-detail Line Value
 *     Detail Line Key : Detail Line Value
 *   Header Line Key : Header Line Value
 *     Detail Line Key : Detail Line Value
 *     Detail Line Key : Detail Line Value
 *       Sub-detail Line Key : Sub-detail Line Value
 *       Sub-detail Line Key : Sub-detail Line Value
 *
 * @author Andrew Keesler <ankeesler1@gmail.com>
 * @date December 15, 2014
 *
 */
public class PacketDisplay {

  private StringBuilder builder = new StringBuilder();

  /**
   * Add a title line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void titleLine(String key, String value) {
    kvp(key, value, 0); // no spaces for a title
  }

  /**
   * Add a header line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void headerLine(String key, String value) {
    kvp(key, value, 2); // 2 spaces for a header
  }

  /**
   * Add a detail line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void detailLine(String key, int value) {
    kvp(key, String.format("%d", value), 4); //  4 spaces for a detail
  }

  /**
   * Add a detail line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void detailLine(String key, String value) {
    kvp(key, value, 4); //  4 spaces for a detail
  }

  /**
   * Add a sub-detail line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void subdetailLine(String key, int value) {
    kvp(key, String.format("%d", value), 6); // 4 spaces for a detail
  }

  /**
   * Add a sub-detail line with the format <it>key: value</it>.
   *
   * @param key The key for the key value pair.
   * @param value The value for the key value pair.
   */
  public void subdetailLine(String key, String value) {
    kvp(key, value, 6); // 6 spaces for a sub-detail
  }


  private void kvp(String key, String value, int spaces) {
    StringBuffer buffer = new StringBuffer(spaces);
    for (int i = 0; i < spaces; i ++)
      buffer.append(' ');
    builder.append(String.format("%s%s: %s\n",
                                 buffer.toString(),
                                 key,
                                 value));
  }
  
  @Override
  public String toString() { return this.builder.toString(); }

}
