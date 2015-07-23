// Generated from cap/SdlLog.g4 by ANTLR 4.2
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class SdlLogLexer extends Lexer {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__4=1, T__3=2, T__2=3, T__1=4, T__0=5, TIMESTAMP=6, FLOAT=7, DIRECTION=8, 
		DATA=9, HEXBYTES=10, HEXBYTE=11, WS=12;
	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	public static final String[] tokenNames = {
		"<INVALID>",
		"'CAPTURE'", "'{'", "')'", "'('", "'}'", "TIMESTAMP", "FLOAT", "DIRECTION", 
		"DATA", "HEXBYTES", "HEXBYTE", "WS"
	};
	public static final String[] ruleNames = {
		"T__4", "T__3", "T__2", "T__1", "T__0", "TIMESTAMP", "FLOAT", "DIRECTION", 
		"DATA", "HEXBYTES", "HEXBYTE", "WS"
	};


	public SdlLogLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "SdlLog.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\2\16]\b\1\4\2\t\2\4"+
		"\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t"+
		"\13\4\f\t\f\4\r\t\r\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\3\3\3\3\4\3\4\3"+
		"\5\3\5\3\6\3\6\3\7\3\7\3\b\6\b/\n\b\r\b\16\b\60\3\b\3\b\6\b\65\n\b\r\b"+
		"\16\b\66\3\t\3\t\3\t\3\t\5\t=\n\t\3\n\3\n\3\n\3\n\3\n\3\n\5\nE\n\n\3\13"+
		"\3\13\3\13\3\13\7\13K\n\13\f\13\16\13N\13\13\3\13\3\13\3\f\3\f\3\f\3\f"+
		"\6\fV\n\f\r\f\16\fW\3\r\3\r\3\r\3\r\2\2\16\3\3\5\4\7\5\t\6\13\7\r\b\17"+
		"\t\21\n\23\13\25\f\27\r\31\16\3\2\5\3\2\62;\4\2\62;CH\5\2\13\f\17\17\""+
		"\"b\2\3\3\2\2\2\2\5\3\2\2\2\2\7\3\2\2\2\2\t\3\2\2\2\2\13\3\2\2\2\2\r\3"+
		"\2\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2\23\3\2\2\2\2\25\3\2\2\2\2\27\3\2\2"+
		"\2\2\31\3\2\2\2\3\33\3\2\2\2\5#\3\2\2\2\7%\3\2\2\2\t\'\3\2\2\2\13)\3\2"+
		"\2\2\r+\3\2\2\2\17.\3\2\2\2\21<\3\2\2\2\23D\3\2\2\2\25L\3\2\2\2\27Q\3"+
		"\2\2\2\31Y\3\2\2\2\33\34\7E\2\2\34\35\7C\2\2\35\36\7R\2\2\36\37\7V\2\2"+
		"\37 \7W\2\2 !\7T\2\2!\"\7G\2\2\"\4\3\2\2\2#$\7}\2\2$\6\3\2\2\2%&\7+\2"+
		"\2&\b\3\2\2\2\'(\7*\2\2(\n\3\2\2\2)*\7\177\2\2*\f\3\2\2\2+,\5\17\b\2,"+
		"\16\3\2\2\2-/\t\2\2\2.-\3\2\2\2/\60\3\2\2\2\60.\3\2\2\2\60\61\3\2\2\2"+
		"\61\62\3\2\2\2\62\64\7\60\2\2\63\65\t\2\2\2\64\63\3\2\2\2\65\66\3\2\2"+
		"\2\66\64\3\2\2\2\66\67\3\2\2\2\67\20\3\2\2\289\7V\2\29=\7Z\2\2:;\7T\2"+
		"\2;=\7Z\2\2<8\3\2\2\2<:\3\2\2\2=\22\3\2\2\2>?\7]\2\2?@\5\25\13\2@A\7_"+
		"\2\2AE\3\2\2\2BC\7]\2\2CE\7_\2\2D>\3\2\2\2DB\3\2\2\2E\24\3\2\2\2FG\5\27"+
		"\f\2GH\7.\2\2HI\7\"\2\2IK\3\2\2\2JF\3\2\2\2KN\3\2\2\2LJ\3\2\2\2LM\3\2"+
		"\2\2MO\3\2\2\2NL\3\2\2\2OP\5\27\f\2P\26\3\2\2\2QR\7\62\2\2RS\7z\2\2SU"+
		"\3\2\2\2TV\t\3\2\2UT\3\2\2\2VW\3\2\2\2WU\3\2\2\2WX\3\2\2\2X\30\3\2\2\2"+
		"YZ\t\4\2\2Z[\3\2\2\2[\\\b\r\2\2\\\32\3\2\2\2\t\2\60\66<DLW\3\b\2\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}