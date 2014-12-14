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
		T__2=1, T__1=2, T__0=3, TIMESTAMP=4, FLOAT=5, DIRECTION=6, DATA=7, HEXBYTES=8, 
		HEXBYTE=9, WS=10;
	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	public static final String[] tokenNames = {
		"<INVALID>",
		"'{'", "'CAPTURE'", "'}'", "TIMESTAMP", "FLOAT", "DIRECTION", "DATA", 
		"HEXBYTES", "HEXBYTE", "WS"
	};
	public static final String[] ruleNames = {
		"T__2", "T__1", "T__0", "TIMESTAMP", "FLOAT", "DIRECTION", "DATA", "HEXBYTES", 
		"HEXBYTE", "WS"
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
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\2\fW\b\1\4\2\t\2\4"+
		"\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t"+
		"\13\3\2\3\2\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\4\3\4\3\5\3\5\3\5\3\5\3"+
		"\6\6\6)\n\6\r\6\16\6*\3\6\3\6\6\6/\n\6\r\6\16\6\60\3\7\3\7\3\7\3\7\5\7"+
		"\67\n\7\3\b\3\b\3\b\3\b\3\b\3\b\5\b?\n\b\3\t\3\t\3\t\3\t\7\tE\n\t\f\t"+
		"\16\tH\13\t\3\t\3\t\3\n\3\n\3\n\3\n\6\nP\n\n\r\n\16\nQ\3\13\3\13\3\13"+
		"\3\13\2\2\f\3\3\5\4\7\5\t\6\13\7\r\b\17\t\21\n\23\13\25\f\3\2\5\3\2\62"+
		";\4\2\62;CH\5\2\13\f\17\17\"\"\\\2\3\3\2\2\2\2\5\3\2\2\2\2\7\3\2\2\2\2"+
		"\t\3\2\2\2\2\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2\23\3\2"+
		"\2\2\2\25\3\2\2\2\3\27\3\2\2\2\5\31\3\2\2\2\7!\3\2\2\2\t#\3\2\2\2\13("+
		"\3\2\2\2\r\66\3\2\2\2\17>\3\2\2\2\21F\3\2\2\2\23K\3\2\2\2\25S\3\2\2\2"+
		"\27\30\7}\2\2\30\4\3\2\2\2\31\32\7E\2\2\32\33\7C\2\2\33\34\7R\2\2\34\35"+
		"\7V\2\2\35\36\7W\2\2\36\37\7T\2\2\37 \7G\2\2 \6\3\2\2\2!\"\7\177\2\2\""+
		"\b\3\2\2\2#$\7*\2\2$%\5\13\6\2%&\7+\2\2&\n\3\2\2\2\')\t\2\2\2(\'\3\2\2"+
		"\2)*\3\2\2\2*(\3\2\2\2*+\3\2\2\2+,\3\2\2\2,.\7\60\2\2-/\t\2\2\2.-\3\2"+
		"\2\2/\60\3\2\2\2\60.\3\2\2\2\60\61\3\2\2\2\61\f\3\2\2\2\62\63\7V\2\2\63"+
		"\67\7Z\2\2\64\65\7T\2\2\65\67\7Z\2\2\66\62\3\2\2\2\66\64\3\2\2\2\67\16"+
		"\3\2\2\289\7]\2\29:\5\21\t\2:;\7_\2\2;?\3\2\2\2<=\7]\2\2=?\7_\2\2>8\3"+
		"\2\2\2><\3\2\2\2?\20\3\2\2\2@A\5\23\n\2AB\7.\2\2BC\7\"\2\2CE\3\2\2\2D"+
		"@\3\2\2\2EH\3\2\2\2FD\3\2\2\2FG\3\2\2\2GI\3\2\2\2HF\3\2\2\2IJ\5\23\n\2"+
		"J\22\3\2\2\2KL\7\62\2\2LM\7z\2\2MO\3\2\2\2NP\t\3\2\2ON\3\2\2\2PQ\3\2\2"+
		"\2QO\3\2\2\2QR\3\2\2\2R\24\3\2\2\2ST\t\4\2\2TU\3\2\2\2UV\b\13\2\2V\26"+
		"\3\2\2\2\t\2*\60\66>FQ\3\b\2\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}