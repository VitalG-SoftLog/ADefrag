LanguageNames=(English=0x409:MSG00409)
LanguageNames=(Russian=0x419:MSG00419)

MessageIdTypedef=VSEDWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(SystemFrom=0x0
               SystemTo  =0xFF
               ADfrag=0x711:FACILITY_AMP_MESSAGE_CODE
              )



;// ====================================================================
;// Categories
;// ====================================================================

MessageId=1
SymbolicName=DFRG_MESSAGE
Language=English
ADfrag message
.
Language=Russian
Сообщение ADfrag
.

;// ====================================================================
;// Messages
;// ====================================================================

;//
;// %1 is reserved by the IO Manager. If IoAllocateErrorLogEntry is
;// called with a device, the name of the device will be inserted into
;// the message at %1. Otherwise, the place of %1 will be left empty.
;// In either case, the insertion strings from the driver's error log
;// entry starts at %2. In other words, the first insertion string goes
;// to %2, the second to %3 and so on.
;//

MessageId=
Facility=ADfrag
Severity=Warning
SymbolicName=DFRG_WARNING
Language=English
%1Message %2
.
Language=Russian
%1Сообщение %2
.

