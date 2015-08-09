// Generated from cap/SdlLog.g4 by ANTLR 4.2
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class SdlLogParser extends Parser {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__4=1, T__3=2, T__2=3, T__1=4, T__0=5, TIMESTAMP=6, FLOAT=7, DIRECTION=8, 
		DATA=9, HEXBYTES=10, HEXBYTE=11, WS=12;
	public static final String[] tokenNames = {
		"<INVALID>", "'('", "')'", "'{'", "'CAPTURE'", "'}'", "TIMESTAMP", "FLOAT", 
		"DIRECTION", "DATA", "HEXBYTES", "HEXBYTE", "WS"
	};
	public static final int
		RULE_capture = 0, RULE_packet = 1;
	public static final String[] ruleNames = {
		"capture", "packet"
	};

	@Override
	public String getGrammarFileName() { return "SdlLog.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }

	public SdlLogParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}
	public static class CaptureContext extends ParserRuleContext {
		public List<PacketContext> packet() {
			return getRuleContexts(PacketContext.class);
		}
		public PacketContext packet(int i) {
			return getRuleContext(PacketContext.class,i);
		}
		public CaptureContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_capture; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof SdlLogListener ) ((SdlLogListener)listener).enterCapture(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof SdlLogListener ) ((SdlLogListener)listener).exitCapture(this);
		}
	}

	public final CaptureContext capture() throws RecognitionException {
		CaptureContext _localctx = new CaptureContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_capture);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(4); match(4);
			setState(5); match(3);
			setState(7); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				{
				setState(6); packet();
				}
				}
				setState(9); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( _la==1 );
			setState(11); match(5);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class PacketContext extends ParserRuleContext {
		public TerminalNode TIMESTAMP() { return getToken(SdlLogParser.TIMESTAMP, 0); }
		public TerminalNode DATA() { return getToken(SdlLogParser.DATA, 0); }
		public TerminalNode DIRECTION() { return getToken(SdlLogParser.DIRECTION, 0); }
		public PacketContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_packet; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof SdlLogListener ) ((SdlLogListener)listener).enterPacket(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof SdlLogListener ) ((SdlLogListener)listener).exitPacket(this);
		}
	}

	public final PacketContext packet() throws RecognitionException {
		PacketContext _localctx = new PacketContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_packet);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(13); match(1);
			setState(14); match(TIMESTAMP);
			setState(15); match(2);
			setState(16); match(DIRECTION);
			setState(17); match(DATA);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static final String _serializedATN =
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3\16\26\4\2\t\2\4\3"+
		"\t\3\3\2\3\2\3\2\6\2\n\n\2\r\2\16\2\13\3\2\3\2\3\3\3\3\3\3\3\3\3\3\3\3"+
		"\3\3\2\2\4\2\4\2\2\24\2\6\3\2\2\2\4\17\3\2\2\2\6\7\7\6\2\2\7\t\7\5\2\2"+
		"\b\n\5\4\3\2\t\b\3\2\2\2\n\13\3\2\2\2\13\t\3\2\2\2\13\f\3\2\2\2\f\r\3"+
		"\2\2\2\r\16\7\7\2\2\16\3\3\2\2\2\17\20\7\3\2\2\20\21\7\b\2\2\21\22\7\4"+
		"\2\2\22\23\7\n\2\2\23\24\7\13\2\2\24\5\3\2\2\2\3\13";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}