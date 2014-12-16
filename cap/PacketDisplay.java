//
// PacketDisplay.java
//
// Andrew Keesler
//
// December 14, 2014
//
// Abstraction of a human-readable packet.

public class PacketDisplay {

  private StringBuilder builder = new StringBuilder();

  public void headerLine(String key, String value) {
    kvp(key, value, true);
  }

  public void detailLine(String key, int value) {
    kvp(key, String.format("%d", value), false);    
  }

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
