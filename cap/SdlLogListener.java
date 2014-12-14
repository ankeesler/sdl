// Generated from cap/SdlLog.g4 by ANTLR 4.2
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link SdlLogParser}.
 */
public interface SdlLogListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link SdlLogParser#capture}.
	 * @param ctx the parse tree
	 */
	void enterCapture(@NotNull SdlLogParser.CaptureContext ctx);
	/**
	 * Exit a parse tree produced by {@link SdlLogParser#capture}.
	 * @param ctx the parse tree
	 */
	void exitCapture(@NotNull SdlLogParser.CaptureContext ctx);

	/**
	 * Enter a parse tree produced by {@link SdlLogParser#packet}.
	 * @param ctx the parse tree
	 */
	void enterPacket(@NotNull SdlLogParser.PacketContext ctx);
	/**
	 * Exit a parse tree produced by {@link SdlLogParser#packet}.
	 * @param ctx the parse tree
	 */
	void exitPacket(@NotNull SdlLogParser.PacketContext ctx);
}