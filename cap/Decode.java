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

public class Decode {

  public static void main(String[] args) {

    // Get the input file.
    if (args.length == 0) {
      System.out.println("usage: java -jar Decode.jar inputFilename");
      return;
    }
    CharStream inputFile = null;
    try {
      inputFile = new ANTLRFileStream(args[0]);
    } catch (Exception e) {
      System.out.println("Error: cannot open file " + args[0] + " for parsing.");
      return;
    }
    
    // Jump up in that parse tree.
    SdlLogLexer lexer = new SdlLogLexer(inputFile);
    CommonTokenStream tokens = new CommonTokenStream(lexer);
    SdlLogParser parser = new SdlLogParser(tokens);
    SdlLogParser.CaptureContext tree = parser.capture();

    ParseTreeWalker walker = new ParseTreeWalker();
    SdlLogIpv6Listener decoder = new SdlLogIpv6Listener();
    walker.walk(decoder, tree);
    
    System.out.println("Done.");
  }
}
