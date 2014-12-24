//
// Ipv6.java
//
// Andrew Keesler
//
// December 23, 2014
//
// Loads and stores the metadata in from YAML.

import java.io.*;
import java.util.*;

import org.yaml.snakeyaml.*;

/** Contains Ipv6 constants. No instances of this class are needed. */
public class Ipv6 {

  private static Map<String, Map<Integer, String>> registry;

  private static final String ICMPV6_TYPES = "icmpv6Types";
  private static final String IPV6_HEADERS = "ipv6Headers";
  private static final String IPV6_HEADER_OPTIONS = "ipv6HeaderOptions";

  /** Don't let users instantiate this class. */
  private Ipv6() { }

  /** Load the yaml stuff from a file. */
  public static void load(String filename) throws Exception {
    new Ipv6().loadInternal(filename);
  }

  @SuppressWarnings("unchecked")
  private void loadInternal(String filename) throws Exception {
    InputStream yamlFile = getClass().getClassLoader().getResourceAsStream(filename);
    registry = (Map<String, Map<Integer, String>>)new Yaml().load(yamlFile);
  }

  /** Get an icmpv6 type from an integer. May return null. */
  public static String icmpv6Type(Integer i) {
    if (registry == null)
      return null;

    return registry.get(ICMPV6_TYPES).get(i);
  }

  /** Get an ipv6 type from an integer. May return null. */
  public static String nextHeader(Integer i) {
    if (registry == null)
      return null;

    return registry.get(IPV6_HEADERS).get(i);
  }

  /** Get the numerical value for a next header string. May return null. */
  public static Integer nextHeader(String name) {
    if (registry == null)
      return null;

    Map<Integer, String> headers = registry.get(IPV6_HEADERS);
    for (Integer i : headers.keySet())
      if (name.equalsIgnoreCase(headers.get(i)))
        return i;
    return null;
  }

  public static String headerOption(Integer i) {
    if (registry == null)
      return null;

    return registry.get(IPV6_HEADER_OPTIONS).get(i);
  }

}