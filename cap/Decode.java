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

  private static final String OPTION_SDL     = "--sdl";
  private static final String OPTION_SDL_KEY = "--sdl-key=";

  private static final String OPTION_IP  = "--ip";

  private static boolean doDecodeSdl(String[] args) {
    for (String arg : args) {
      if (arg.equals(OPTION_SDL)) {
        return true;
      }
    }
    return false;
  }

  private static boolean doDecodeIp(String[] args) {
    for (String arg : args) {
      if (arg.equals(OPTION_IP)) {
        return true;
      }
    }
    return false;
  }

  private static String sdlKey(String[] args) {
    for (String arg : args) {
      if (arg.startsWith(OPTION_SDL_KEY)) {
        return arg.substring(OPTION_SDL_KEY.length());
      }
    }
    return null;
  }

  public static void main(String[] args) {
    System.out.println(" @ Starting...");

    // Get the input file.
    if (args.length == 0) {
      String usage = String.format("java -jar Decoder.jar inputFilename [%s] [%s] [%s]", OPTION_SDL, OPTION_IP);
      System.out.println(" @ Error: incorrect usage.");
      System.out.println(" @ Usage: " + usage);
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
    System.out.println(" @ ...ready.");

    // SDL decoding.
    if (doDecodeSdl(args)) {
      walker.walk(new SdlLogSdlListener(sdlKey(args)), tree);
    }

    // IPv6 decoding.
    if (doDecodeIp(args)) {
      walker.walk(new SdlLogIpv6Listener(), tree);
    }
    
    System.out.println(" @ Done.");
  }
}
