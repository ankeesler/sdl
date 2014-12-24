//
// Decode.java
//
// Andrew Keeler
//
// December 13, 2014
//
// Java - it's just the worst.

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import java.io.File;

public class Decode {

  private static final String YAML_FILENAME = "ipv6.yaml";

  public static void main(String[] args) {
    System.out.println(" @ Starting...");

    // Get the input file.
    if (args.length == 0) {
      System.out.println(" @ Error: incorrect usage.");
      System.out.println(" @ Usage: java -jar Decode.jar inputFilename");
      System.out.println(" @ Exiting.");
      return;
    }
    CharStream inputFile = null;
    try {
      inputFile = new ANTLRFileStream(args[0]);
    } catch (Exception e) {
      System.out.println(" @ Error: cannot open file " + args[0] + " for parsing.");
      System.out.println(" @ Exiting.");
      return;
    }

    // Load the constants.
    boolean fail = false;
    try {
      Ipv6.load(YAML_FILENAME);
    } catch (Exception e) {
      System.out.println(" @ Warning: yaml file cannot be loaded.");
      fail = true;
    }
    if (!fail)
      System.out.println(" @ ...loaded IPv6 constants...");
    
    // Jump up in that parse tree.
    SdlLogLexer lexer = new SdlLogLexer(inputFile);
    CommonTokenStream tokens = new CommonTokenStream(lexer);
    SdlLogParser parser = new SdlLogParser(tokens);
    SdlLogParser.CaptureContext tree = parser.capture();
    System.out.println(" @ ...parsed log...");

    ParseTreeWalker walker = new ParseTreeWalker();
    SdlLogIpv6Listener decoder = new SdlLogIpv6Listener();
    System.out.println(" @ ...ready.");

    // Perform the decoding.
    walker.walk(decoder, tree);
    
    System.out.println(" @ Done.");
  }
}
