#include "QtPlugin.h"
#include "pluginmain.h"
#include "SnowmanView.h"
#include <QFile>

static SnowmanView* snowman;
static HANDLE hSetupEvent;
static HANDLE hStopEvent;

static QByteArray getResourceBytes(const char* path)
{
	QByteArray b;
	QFile s(path);
	if(s.open(QFile::ReadOnly))
		b = s.readAll();
	return b;
}

static QWidget* getParent()
{
    return QWidget::find((WId)Plugin::hwndDlg);
}

void QtPlugin::Init()
{
	hSetupEvent = CreateEventW(nullptr, true, false, nullptr);
	hStopEvent = CreateEventW(nullptr, true, false, nullptr);
}

enum
{
	MENU_DISASM_DECOMPILE_SELECTION,
	MENU_DISASM_DECOMPILE_FUNCTION,
	MENU_GRAPH_DECOMPILE,
};

void QtPlugin::Setup()
{
    QWidget* parent = getParent();
	snowman = new SnowmanView(parent);
	snowman->setWindowTitle("Snowman");
	snowman->setWindowIcon(QIcon(":/icons/images/snowman.png"));
    GuiAddQWidgetTab(snowman);

	auto snowmanBytes = getResourceBytes(":/icons/images/snowman.png");
	auto selectionBytes = getResourceBytes(":/icons/images/decompile_selection.png");
	auto functionBytes = getResourceBytes(":/icons/images/decompile_function.png");

	ICONDATA snowmanIcon{ snowmanBytes.data(), snowmanBytes.size() };
	ICONDATA selectionIcon{ selectionBytes.data(), selectionBytes.size() };
	ICONDATA functionIcon{ functionBytes.data(), functionBytes.size() };

	_plugin_menuseticon(Plugin::hMenuDisasm, &snowmanIcon);
	_plugin_menuaddentry(Plugin::hMenuDisasm, MENU_DISASM_DECOMPILE_SELECTION, "Decompile selection");
	_plugin_menuentrysethotkey(Plugin::handle, MENU_DISASM_DECOMPILE_SELECTION, "Shift+F5");
	_plugin_menuentryseticon(Plugin::handle, MENU_DISASM_DECOMPILE_SELECTION, &selectionIcon);
	_plugin_menuaddentry(Plugin::hMenuDisasm, MENU_DISASM_DECOMPILE_FUNCTION, "Decompile function");
	_plugin_menuentrysethotkey(Plugin::handle, MENU_DISASM_DECOMPILE_FUNCTION, "F5");
	_plugin_menuentryseticon(Plugin::handle, MENU_DISASM_DECOMPILE_FUNCTION, &functionIcon);
	_plugin_menuentrysetvisible(Plugin::handle, MENU_DISASM_DECOMPILE_FUNCTION, false);
	
	_plugin_menuseticon(Plugin::hMenuGraph, &snowmanIcon);
	_plugin_menuaddentry(Plugin::hMenuGraph, MENU_GRAPH_DECOMPILE, "Decompile");
	_plugin_menuentrysethotkey(Plugin::handle, MENU_GRAPH_DECOMPILE, "Tab");
	_plugin_menuentryseticon(Plugin::handle, MENU_GRAPH_DECOMPILE, &functionIcon);

	SetEvent(hSetupEvent);
}

void QtPlugin::WaitForSetup()
{
	WaitForSingleObject(hSetupEvent, INFINITE);
}

void QtPlugin::Stop()
{
    GuiCloseQWidgetTab(snowman);
	snowman->close();
    delete snowman;
	SetEvent(hStopEvent);
}

void QtPlugin::WaitForStop()
{
	WaitForSingleObject(hStopEvent, INFINITE);
}

void QtPlugin::ShowTab()
{
    GuiShowQWidgetTab(snowman);
}

void QtPlugin::MenuPrepare(int hMenu)
{
	if(hMenu == GUI_DISASM_MENU)
	{
		SELECTIONDATA sel = { 0, 0 };
		GuiSelectionGet(GUI_DISASSEMBLY, &sel);
		auto selectionInFunction = DbgFunctionGet(sel.start, nullptr, nullptr);
		_plugin_menuentrysetvisible(Plugin::handle, MENU_DISASM_DECOMPILE_FUNCTION, selectionInFunction);
	}
}

void QtPlugin::MenuEntry(int hEntry)
{
	switch(hEntry)
	{
	case MENU_DISASM_DECOMPILE_SELECTION:
	{
		if(!DbgIsDebugging())
			return;

		SELECTIONDATA sel = { 0, 0 };
		GuiSelectionGet(GUI_DISASSEMBLY, &sel);

		ShowTab();
		SnowmanRange range{ sel.start, sel.end + 1 };
		snowman->decompileAt(&range, 1);
	}
	break;

	case MENU_DISASM_DECOMPILE_FUNCTION:
	{
		if(!DbgIsDebugging())
			return;

		SELECTIONDATA sel = { 0, 0 };
		GuiSelectionGet(GUI_DISASSEMBLY, &sel);
		duint addr = sel.start;
		duint start;
		duint end;
		if(DbgFunctionGet(addr, &start, &end))
		{
			BASIC_INSTRUCTION_INFO info;
			DbgDisasmFastAt(end, &info);
			end += info.size - 1;

			ShowTab();
			SnowmanRange range{ start,end };
			snowman->decompileAt(&range, 1);
		}
	}
	break;

	case MENU_GRAPH_DECOMPILE:
	{
		if(!DbgIsDebugging())
			return;

		BridgeCFGraphList graphList;
		GuiGetCurrentGraph(&graphList);
		BridgeCFGraph currentGraph(&graphList, true);
		if(currentGraph.nodes.empty())
			return;

		std::vector<SnowmanRange> ranges;
		ranges.reserve(currentGraph.nodes.size());
		for(const auto & nodeIt : currentGraph.nodes)
		{
			SnowmanRange r;
			const BridgeCFNode & node = nodeIt.second;
			r.start = node.instrs.empty() ? node.start : node.instrs[0].addr;
			r.end = node.instrs.empty() ? node.end : node.instrs[node.instrs.size() - 1].addr;
			BASIC_INSTRUCTION_INFO info;
			DbgDisasmFastAt(r.end, &info);
			r.end += info.size - 1;
			ranges.push_back(r);
		}
		std::sort(ranges.begin(), ranges.end(), [](const SnowmanRange & a, const SnowmanRange & b)
		{
			return a.start > b.start;
		});

		ShowTab();
		snowman->decompileAt(ranges.data(), ranges.size());
	}
	break;
	}
}