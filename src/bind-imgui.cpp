#include "imgui.h"

// Forward-declare internal API used by demo (defined in imgui_widgets.cpp)
namespace ImGui { IMGUI_API void TreeNodeSetOpen(ImGuiID storage_id, bool open); }

#ifndef __FLT_MAX__
#define __FLT_MAX__ 3.40282346638528859812e+38F
#endif

// Stubs for ShowStyleSelector/ShowFontSelector no longer needed in v1.92
// (imgui_demo.cpp provides stubs when IMGUI_DISABLE_DEMO_WINDOWS, ShowFontSelector moved to imgui.cpp)

#include <emscripten/bind.h>

#define FUNCTION(RET, ARGS, CODE...) \
    emscripten::optional_override([] ARGS -> RET { CODE })

#define CLASS_MEMBER(CLASS, MEMBER) \
    .property(#MEMBER, &CLASS::MEMBER)

#define CLASS_MEMBER_GET(CLASS, MEMBER, GET) \
    .property(#MEMBER, FUNCTION(emscripten::val, (const CLASS& that), GET))

#define CLASS_MEMBER_SET(CLASS, MEMBER, SET) \
    .property(#MEMBER, NULL, FUNCTION(void, (CLASS& that, emscripten::val value), SET))

#define CLASS_MEMBER_GET_SET(CLASS, MEMBER, GET, SET) \
    .property(#MEMBER, \
        FUNCTION(emscripten::val, (const CLASS& that), GET), \
        FUNCTION(void, (CLASS& that, emscripten::val value), SET))

#define CLASS_MEMBER_GET_RAW_POINTER(CLASS, MEMBER) \
    .property(#MEMBER, FUNCTION(emscripten::val, (const CLASS& that), { \
        auto p = that.MEMBER; return p == NULL ? emscripten::val::null() : emscripten::val(p); \
    }))

#define CLASS_MEMBER_GET_SET_RAW_POINTER(CLASS, MEMBER) \
    .property(#MEMBER, FUNCTION(emscripten::val, (const CLASS& that), { \
        auto p = that.MEMBER; return p == NULL ? emscripten::val::null() : emscripten::val(p); \
    }), FUNCTION(void, (CLASS& that, emscripten::val value), { \
        that.MEMBER = value.isNull() ? NULL : value.as<decltype(that.MEMBER)>(emscripten::allow_raw_pointers()); \
    }))

#define CLASS_MEMBER_GET_RAW_REFERENCE(CLASS, MEMBER) \
    .property(#MEMBER, FUNCTION(emscripten::val, (const CLASS& that), { \
        auto p = &that.MEMBER; return emscripten::val(p); \
    }))

#define CLASS_METHOD(CLASS, METHOD) \
    .function(#METHOD, &CLASS::METHOD)

// Global storage for drag/drop payload data (JS side-channel)
static emscripten::val g_DragDropPayloadData = emscripten::val::null();

#include <malloc.h>

emscripten::val get_mallinfo() {
    const auto& i = mallinfo();
    emscripten::val rv(emscripten::val::object());
    rv.set("arena", emscripten::val(i.arena));
    rv.set("ordblks", emscripten::val(i.ordblks));
    rv.set("smblks", emscripten::val(i.smblks));
    rv.set("hblks", emscripten::val(i.hblks));
    rv.set("hblkhd", emscripten::val(i.hblkhd));
    rv.set("usmblks", emscripten::val(i.usmblks));
    rv.set("fsmblks", emscripten::val(i.fsmblks));
    rv.set("uordblks", emscripten::val(i.uordblks));
    rv.set("fordblks", emscripten::val(i.fordblks));
    rv.set("keepcost", emscripten::val(i.keepcost));
    return rv;
}

EMSCRIPTEN_BINDINGS(mallinfo) {
    emscripten::function("mallinfo", &get_mallinfo);
}

#define TODO() printf("TODO: %s\n", __PRETTY_FUNCTION__)

class WrapImGuiContext {
private:
    static WrapImGuiContext* _current_wrap;
public:
    static WrapImGuiContext* CreateContext(ImFontAtlas* shared_font_atlas = NULL) {
        return new WrapImGuiContext(shared_font_atlas);
    }
    static void DestroyContext(WrapImGuiContext* wrap) {
        delete wrap;
    }
    static void SetCurrentContext(WrapImGuiContext* wrap) {
        _current_wrap = wrap;
        ImGui::SetCurrentContext(wrap == NULL ? NULL : wrap->ctx);
    }
    static WrapImGuiContext* GetCurrentContext() {
        return _current_wrap;
    }
private:
    static const char* _GetClipboardText(ImGuiContext* ctx) {
        WrapImGuiContext* wrap_ctx = WrapImGuiContext::GetCurrentContext();
        if (!wrap_ctx->_ImGuiIO_GetClipboardTextFn.isNull()) {
            wrap_ctx->_ImGuiIO_ClipboardText = wrap_ctx->_ImGuiIO_GetClipboardTextFn(wrap_ctx->_ImGuiIO_ClipboardUserData).as<std::string>();
        }
        return wrap_ctx->_ImGuiIO_ClipboardText.c_str();
    }
    static void _SetClipboardText(ImGuiContext* ctx, const char* text) {
        WrapImGuiContext* wrap_ctx = WrapImGuiContext::GetCurrentContext();
        wrap_ctx->_ImGuiIO_ClipboardText = text;
        if (!wrap_ctx->_ImGuiIO_SetClipboardTextFn.isNull()) {
            wrap_ctx->_ImGuiIO_SetClipboardTextFn(wrap_ctx->_ImGuiIO_ClipboardUserData, wrap_ctx->_ImGuiIO_ClipboardText);
        }
    }
private:
    ImGuiContext* ctx;
public:
    std::string _ImGuiIO_IniFilename = "";
    std::string _ImGuiIO_LogFilename = "";
    emscripten::val _ImGuiIO_UserData = emscripten::val::null();
    emscripten::val _ImGuiIO_BackendPlatformName = emscripten::val::null();
    emscripten::val _ImGuiIO_BackendRendererName = emscripten::val::null();
    emscripten::val _ImGuiIO_BackendPlatformUserData = emscripten::val::null();
    emscripten::val _ImGuiIO_BackendRendererUserData = emscripten::val::null();
    emscripten::val _ImGuiIO_BackendLanguageUserData = emscripten::val::null();
    std::string _ImGuiIO_ClipboardText = "";
    emscripten::val _ImGuiIO_GetClipboardTextFn = emscripten::val::null();
    emscripten::val _ImGuiIO_SetClipboardTextFn = emscripten::val::null();
    emscripten::val _ImGuiIO_ClipboardUserData = emscripten::val::null();

    emscripten::val _ImGui_SetNextWindowSizeConstraints_custom_callback = emscripten::val::undefined();

    emscripten::val _ImGui_PlotLines_values_getter = emscripten::val::undefined();
    emscripten::val _ImGui_PlotLines_data = emscripten::val::undefined();

    emscripten::val _ImGui_PlotHistogram_values_getter = emscripten::val::undefined();
    emscripten::val _ImGui_PlotHistogram_data = emscripten::val::undefined();

    emscripten::val _ImGui_Combo_items_getter = emscripten::val::undefined();
    emscripten::val _ImGui_Combo_data = emscripten::val::undefined();
    int _ImGui_Combo_items_count = 0;
    std::string _ImGui_Combo_text = "";

    emscripten::val _ImGui_InputText_callback = emscripten::val::undefined();

    emscripten::val _ImGui_InputTextMultiline_callback = emscripten::val::undefined();

    emscripten::val _ImGui_ListBox_A_items = emscripten::val::undefined();
    int _ImGui_ListBox_A_items_count = 0;
    std::string _ImGui_ListBox_A_text = "";

    emscripten::val _ImGui_ListBox_B_items_getter = emscripten::val::undefined();
    emscripten::val _ImGui_ListBox_B_data = emscripten::val::undefined();
    int _ImGui_ListBox_B_items_count = 0;
    std::string _ImGui_ListBox_B_text = "";

    emscripten::val _ImGui_SetAllocatorFunctions_alloc_func = emscripten::val::undefined();
    emscripten::val _ImGui_SetAllocatorFunctions_free_func = emscripten::val::undefined();
    emscripten::val _ImGui_SetAllocatorFunctions_user_data = emscripten::val::undefined();

public:
    WrapImGuiContext(ImFontAtlas* shared_font_atlas = NULL): ctx(ImGui::CreateContext(shared_font_atlas)) {
        ImGuiContext* prev_ctx = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(ctx);
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.LogFilename = NULL;
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_GetClipboardTextFn = WrapImGuiContext::_GetClipboardText;
        platform_io.Platform_SetClipboardTextFn = WrapImGuiContext::_SetClipboardText;
        platform_io.Platform_ClipboardUserData = NULL;
        ImGui::SetCurrentContext(prev_ctx);
    }
    ~WrapImGuiContext() {
        ImGuiContext* prev_ctx = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(ctx);
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.LogFilename = NULL;
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_GetClipboardTextFn = NULL;
        platform_io.Platform_SetClipboardTextFn = NULL;
        platform_io.Platform_ClipboardUserData = NULL;
        ImGui::SetCurrentContext(prev_ctx);
        ImGui::DestroyContext(ctx);
        ctx = NULL;
    }
};

WrapImGuiContext* WrapImGuiContext::_current_wrap = NULL;

EMSCRIPTEN_BINDINGS(WrapImGuiContext) {
    emscripten::class_<WrapImGuiContext>("WrapImGuiContext")
    ;
}

template <typename T>
T import_value(const emscripten::val& value) {
    return value.as<T>();
}

template <typename T>
emscripten::val export_value(const T& value) {
    return emscripten::val(value);
}

template <>
float import_value(const emscripten::val& value) {
    // return __import_float(value);
    const double _value = value.as<double>();
    if (double(+FLT_MAX) <= _value) return +FLT_MAX;
    if (_value <= double(-FLT_MAX)) return -FLT_MAX;
    return float(_value);
}

template <typename T>
class import_maybe_null_value {
protected:
    T _value;
    const emscripten::val& _import;
public:
    import_maybe_null_value(const emscripten::val& _import) : _import(_import) {
        if (!_import.isNull()) { _import_value(); }
    }
    virtual void _import_value() {
        _value = import_value<T>(_import);
    }
    operator T*() { return _import.isNull() ? NULL : &_value; }
    operator const T*() const { return _import.isNull() ? NULL : &_value; }
};

class import_maybe_null_string : public import_maybe_null_value<std::string> {
public:
    import_maybe_null_string(const emscripten::val& _import) : import_maybe_null_value(_import) {}
    operator const char*() const { return _import.isNull() ? NULL : _value.c_str(); }
};

emscripten::val export_maybe_null_string(const char* value) {
    return value == NULL ? emscripten::val::null() : emscripten::val(value);
}

emscripten::val export_ImGuiSelectionRequest(const ImGuiSelectionRequest& request) {
    emscripten::val out = emscripten::val::object();
    out.set("Type", emscripten::val((int)request.Type));
    out.set("Selected", emscripten::val(request.Selected));
    out.set("RangeDirection", emscripten::val((int)request.RangeDirection));
    out.set("RangeFirstItem", emscripten::val((double)request.RangeFirstItem));
    out.set("RangeLastItem", emscripten::val((double)request.RangeLastItem));
    return out;
}

template <typename T, size_t N = 1>
class access_value {
protected:
    T _value[N];
    emscripten::val& _access;
public:
    access_value(emscripten::val& _access) : _access(_access) {
        _import_value();
    }
    virtual ~access_value() {
        _export_value();
    }
    virtual void _import_value() {
        for (size_t i = 0; i < N; ++i) {
            _value[i] = import_value<T>(_access[i]);
        }
    }
    virtual void _export_value() {
        for (size_t i = 0; i < N; ++i) {
            _access.set(i, export_value<T>(_value[i]));
        }
    }
    operator T&() { return _value[0]; }
    operator const T&() const { return _value[0]; }
    operator T*() { return &_value[0]; }
    operator const T*() const { return &_value[0]; }
};

template <typename T, size_t N = 1>
class access_maybe_null_value {
protected:
    T _value[N];
    emscripten::val& _access;
public:
    access_maybe_null_value(emscripten::val& _access) : _access(_access) {
        if (!_access.isNull()) { _import_value(); }
    }
    virtual ~access_maybe_null_value() {
        if (!_access.isNull()) { _export_value(); }
    }
    virtual void _import_value() {
        for (size_t i = 0; i < N; ++i) {
            _value[i] = import_value<T>(_access[i]);
        }
    }
    virtual void _export_value() {
        for (size_t i = 0; i < N; ++i) {
            _access.set(i, export_value<T>(_value[i]));
        }
    }
    operator T*() { return _access.isNull() ? NULL : &_value[0]; }
    operator const T*() const { return _access.isNull() ? NULL : &_value[0]; }
};

template <typename T>
class access_typed_array {
protected:
    std::vector<T> _value;
    emscripten::val& _access;
public:
    access_typed_array(emscripten::val& access): _access(access) {
        _value.resize(_access["length"].template as<size_t>());
        emscripten::val(emscripten::typed_memory_view<T>(_value.size(), _value.data())).call<void>("set", _access);
    }
    ~access_typed_array() {
        _access.call<void>("set", emscripten::typed_memory_view<T>(_value.size(), _value.data()));
    }
    T* data() { return _value.data(); }
    const T* data() const { return _value.data(); }
    size_t size() { return _value.size(); }
    operator std::vector<T>&() { return _value; }
    operator const std::vector<T>&() const { return _value; }
};

ImVec2& import_ImVec2(const emscripten::val& value, ImVec2& out) {
    out.x = import_value<float>(value["x"]);
    out.y = import_value<float>(value["y"]);
    return out;
}

ImVec2 import_ImVec2(const emscripten::val& value) {
    ImVec2 out; import_ImVec2(value, out); return out;
}

emscripten::val export_ImVec2(const ImVec2& value, emscripten::val out) {
    out.set("x", export_value<float>(value.x));
    out.set("y", export_value<float>(value.y));
    return out;
}

emscripten::val export_ImVec2(const ImVec2& value) {
    return export_ImVec2(value, emscripten::val::object());
}

emscripten::val export_ImGuiPlatformMonitor(const ImGuiPlatformMonitor& monitor) {
    emscripten::val out = emscripten::val::object();
    out.set("MainPos", export_ImVec2(monitor.MainPos));
    out.set("MainSize", export_ImVec2(monitor.MainSize));
    out.set("WorkPos", export_ImVec2(monitor.WorkPos));
    out.set("WorkSize", export_ImVec2(monitor.WorkSize));
    out.set("DpiScale", emscripten::val(monitor.DpiScale));
    out.set("PlatformHandle", monitor.PlatformHandle == NULL ? emscripten::val::null() : emscripten::val((intptr_t)monitor.PlatformHandle));
    return out;
}

emscripten::val export_ImFontGlyph(const ImFontGlyph& glyph) {
    emscripten::val out = emscripten::val::object();
    out.set("Codepoint", emscripten::val((unsigned int)glyph.Codepoint));
    out.set("Visible", emscripten::val(glyph.Visible));
    out.set("AdvanceX", emscripten::val(glyph.AdvanceX));
    out.set("X0", emscripten::val(glyph.X0));
    out.set("Y0", emscripten::val(glyph.Y0));
    out.set("X1", emscripten::val(glyph.X1));
    out.set("Y1", emscripten::val(glyph.Y1));
    out.set("U0", emscripten::val(glyph.U0));
    out.set("V0", emscripten::val(glyph.V0));
    out.set("U1", emscripten::val(glyph.U1));
    out.set("V1", emscripten::val(glyph.V1));
    return out;
}

emscripten::val export_ImFontAtlasRect(const ImFontAtlasRect& rect) {
    emscripten::val out = emscripten::val::object();
    out.set("x", emscripten::val((int)rect.x));
    out.set("y", emscripten::val((int)rect.y));
    out.set("w", emscripten::val((int)rect.w));
    out.set("h", emscripten::val((int)rect.h));
    out.set("uv0", export_ImVec2(rect.uv0));
    out.set("uv1", export_ImVec2(rect.uv1));
    return out;
}

template <>
ImVec2 import_value(const emscripten::val& value) {
    return import_ImVec2(value);
}

emscripten::val ImVec2_Set(emscripten::val that, emscripten::val x, emscripten::val y) {
    that.set("x", x);
    that.set("y", y);
    return that;
}

emscripten::val ImVec2_Copy(emscripten::val that, emscripten::val other) {
    that.set("x", other["x"]);
    that.set("y", other["y"]);
    return that;
}

bool ImVec2_Equals(const emscripten::val that, emscripten::val other) {
    if (!that["x"].strictlyEquals(other["x"])) { return false; }
    if (!that["y"].strictlyEquals(other["y"])) { return false; }
    return true;
}

EMSCRIPTEN_BINDINGS(ImVec2) {
    emscripten::class_<ImVec2>("ImVec2")
        // no constructors for EmscriptenClassReference
        // .constructor()
        // .constructor<float, float>()
        CLASS_MEMBER(ImVec2, x)
        CLASS_MEMBER(ImVec2, y)
        .function("Set", &ImVec2_Set)
        .function("Copy", &ImVec2_Copy)
        .function("Equals", &ImVec2_Equals)
    ;
}

ImVec4& import_ImVec4(const emscripten::val& value, ImVec4& out) {
    out.x = import_value<float>(value["x"]);
    out.y = import_value<float>(value["y"]);
    out.z = import_value<float>(value["z"]);
    out.w = import_value<float>(value["w"]);
    return out;
}

ImVec4 import_ImVec4(const emscripten::val& value) {
    ImVec4 out; import_ImVec4(value, out); return out;
}

emscripten::val export_ImVec4(const ImVec4& value, emscripten::val out) {
    out.set("x", export_value<float>(value.x));
    out.set("y", export_value<float>(value.y));
    out.set("z", export_value<float>(value.z));
    out.set("w", export_value<float>(value.w));
    return out;
}

emscripten::val export_ImVec4(const ImVec4& value) {
    return export_ImVec4(value, emscripten::val::object());
}

template <>
ImVec4 import_value(const emscripten::val& value) {
    return import_ImVec4(value);
}

emscripten::val ImVec4_Set(emscripten::val that, const emscripten::val x, const emscripten::val y, const emscripten::val z, const emscripten::val w) {
    that.set("x", x);
    that.set("y", y);
    that.set("z", z);
    that.set("w", w);
    return that;
}

emscripten::val ImVec4_Copy(emscripten::val that, emscripten::val other) {
    that.set("x", other["x"]);
    that.set("y", other["y"]);
    that.set("z", other["z"]);
    that.set("w", other["w"]);
    return that;
}

bool ImVec4_Equals(const emscripten::val that, emscripten::val other) {
    if (!that["x"].strictlyEquals(other["x"])) { return false; }
    if (!that["y"].strictlyEquals(other["y"])) { return false; }
    if (!that["z"].strictlyEquals(other["z"])) { return false; }
    if (!that["w"].strictlyEquals(other["w"])) { return false; }
    return true;
}

EMSCRIPTEN_BINDINGS(ImVec4) {
    emscripten::class_<ImVec4>("ImVec4")
        // no constructors for EmscriptenClassReference
        // .constructor()
        // .constructor<float, float, float, float>()
        CLASS_MEMBER(ImVec4, x)
        CLASS_MEMBER(ImVec4, y)
        CLASS_MEMBER(ImVec4, z)
        CLASS_MEMBER(ImVec4, w)
        .function("Set", &ImVec4_Set)
        .function("Copy", &ImVec4_Copy)
        .function("Equals", &ImVec4_Equals)
    ;
}

// Shared state of InputText(), passed to callback when a ImGuiInputTextFlags_Callback* flag is used and the corresponding callback is triggered.
// struct ImGuiInputTextCallbackData
EMSCRIPTEN_BINDINGS(ImGuiInputTextCallbackData) {
    emscripten::class_<ImGuiInputTextCallbackData>("ImGuiInputTextCallbackData")
        // ImGuiInputTextFlags EventFlag;      // One of ImGuiInputTextFlags_Callback* // Read-only
        CLASS_MEMBER(ImGuiInputTextCallbackData, EventFlag)
        // ImGuiInputTextFlags Flags;          // What user passed to InputText()      // Read-only
        CLASS_MEMBER(ImGuiInputTextCallbackData, Flags)
        // void*               UserData;       // What user passed to InputText()      // Read-only
        // bool                ReadOnly;       // Read-only mode                       // Read-only
        // CLASS_MEMBER(ImGuiInputTextCallbackData, ReadOnly)

        // CharFilter event:
        // ImWchar             EventChar;      // Character input                      // Read-write (replace character or set to zero)
        CLASS_MEMBER(ImGuiInputTextCallbackData, EventChar)

        // Completion,History,Always events:
        // If you modify the buffer contents make sure you update 'BufTextLen' and set 'BufDirty' to true.
        // ImGuiKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only
        CLASS_MEMBER(ImGuiInputTextCallbackData, EventKey)
        // char*               Buf;            // Current text buffer                  // Read-write (pointed data only, can't replace the actual pointer)
        CLASS_MEMBER_GET_SET(ImGuiInputTextCallbackData, Buf, 
            { return emscripten::val(std::string(that.Buf)); },
            { strncpy(that.Buf, value.as<std::string>().c_str(), that.BufSize - 1); }
        )
        // int                 BufTextLen;     // Current text length in bytes         // Read-write
        CLASS_MEMBER(ImGuiInputTextCallbackData, BufTextLen)
        // int                 BufSize;        // Maximum text length in bytes         // Read-only
        CLASS_MEMBER(ImGuiInputTextCallbackData, BufSize)
        // bool                BufDirty;       // Set if you modify Buf/BufTextLen!!   // Write
        CLASS_MEMBER(ImGuiInputTextCallbackData, BufDirty)
        // int                 CursorPos;      //                                      // Read-write
        CLASS_MEMBER(ImGuiInputTextCallbackData, CursorPos)
        // int                 SelectionStart; //                                      // Read-write (== to SelectionEnd when no selection)
        CLASS_MEMBER(ImGuiInputTextCallbackData, SelectionStart)
        // int                 SelectionEnd;   //                                      // Read-write
        CLASS_MEMBER(ImGuiInputTextCallbackData, SelectionEnd)

        // NB: Helper functions for text manipulation. Calling those function loses selection.
        // IMGUI_API void    DeleteChars(int pos, int bytes_count);
        CLASS_METHOD(ImGuiInputTextCallbackData, DeleteChars)
        // IMGUI_API void    InsertChars(int pos, const char* text, const char* text_end = NULL);
        .function("InsertChars", FUNCTION(void, (ImGuiInputTextCallbackData& that, int pos, std::string text), {
            that.InsertChars(pos, text.c_str(), NULL);
        }))
        // bool              HasSelection() const { return SelectionStart != SelectionEnd; }
        CLASS_METHOD(ImGuiInputTextCallbackData, HasSelection)
    ;
}

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
// struct ImGuiSizeCallbackData
// {
//     void*   UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints()
//     ImVec2  Pos;            // Read-only.   Window position, for reference.
//     ImVec2  CurrentSize;    // Read-only.   Current window size.
//     ImVec2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
// };
EMSCRIPTEN_BINDINGS(ImGuiSizeCallbackData) {
    emscripten::class_<ImGuiSizeCallbackData>("ImGuiSizeCallbackData")
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiSizeCallbackData, Pos)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiSizeCallbackData, CurrentSize)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiSizeCallbackData, DesiredSize)
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiListClipper) {
    emscripten::class_<ImGuiListClipper>("ImGuiListClipper")
        .constructor()
        CLASS_MEMBER(ImGuiListClipper, DisplayStart)
        CLASS_MEMBER(ImGuiListClipper, DisplayEnd)
        CLASS_MEMBER(ImGuiListClipper, ItemsCount)
        CLASS_MEMBER(ImGuiListClipper, ItemsHeight)
        CLASS_METHOD(ImGuiListClipper, Begin)
        CLASS_METHOD(ImGuiListClipper, End)
        CLASS_METHOD(ImGuiListClipper, Step)
        .function("IncludeItemByIndex", &ImGuiListClipper::IncludeItemByIndex)
        .function("IncludeItemsByIndex", &ImGuiListClipper::IncludeItemsByIndex)
        .function("SeekCursorForItem", &ImGuiListClipper::SeekCursorForItem)
        CLASS_MEMBER(ImGuiListClipper, UserIndex)
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiStorage) {
    emscripten::class_<ImGuiStorage>("ImGuiStorage")
        .constructor()
        .function("Clear", FUNCTION(void, (ImGuiStorage& that), { that.Clear(); }))
        .function("GetInt", FUNCTION(int, (const ImGuiStorage& that, ImGuiID key, int default_val), { return that.GetInt(key, default_val); }))
        .function("SetInt", FUNCTION(void, (ImGuiStorage& that, ImGuiID key, int val), { that.SetInt(key, val); }))
        .function("GetBool", FUNCTION(bool, (const ImGuiStorage& that, ImGuiID key, bool default_val), { return that.GetBool(key, default_val); }))
        .function("SetBool", FUNCTION(void, (ImGuiStorage& that, ImGuiID key, bool val), { that.SetBool(key, val); }))
        .function("GetFloat", FUNCTION(float, (const ImGuiStorage& that, ImGuiID key, float default_val), { return that.GetFloat(key, default_val); }))
        .function("SetFloat", FUNCTION(void, (ImGuiStorage& that, ImGuiID key, float val), { that.SetFloat(key, val); }))
        .function("GetVoidPtr", FUNCTION(emscripten::val, (const ImGuiStorage& that, ImGuiID key), {
            void* p = that.GetVoidPtr(key);
            return p == NULL ? emscripten::val::null() : emscripten::val((intptr_t)p);
        }))
        .function("SetVoidPtr", FUNCTION(void, (ImGuiStorage& that, ImGuiID key, emscripten::val value), {
            that.SetVoidPtr(key, value.isNull() ? NULL : (void*)(intptr_t)value.as<intptr_t>());
        }))
        .function("BuildSortByKey", FUNCTION(void, (ImGuiStorage& that), { that.BuildSortByKey(); }))
        .function("SetAllInt", FUNCTION(void, (ImGuiStorage& that, int val), { that.SetAllInt(val); }))
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiWindowClass) {
    emscripten::class_<ImGuiWindowClass>("ImGuiWindowClass")
        .constructor()
        CLASS_MEMBER(ImGuiWindowClass, ClassId)
        CLASS_MEMBER(ImGuiWindowClass, ParentViewportId)
        CLASS_MEMBER(ImGuiWindowClass, ViewportFlagsOverrideSet)
        CLASS_MEMBER(ImGuiWindowClass, ViewportFlagsOverrideClear)
        CLASS_MEMBER(ImGuiWindowClass, TabItemFlagsOverrideSet)
        CLASS_MEMBER(ImGuiWindowClass, DockNodeFlagsOverrideSet)
        CLASS_MEMBER(ImGuiWindowClass, DockingAlwaysTabBar)
        CLASS_MEMBER(ImGuiWindowClass, DockingAllowUnclassed)
    ;
}

ImGuiWindowClass& import_ImGuiWindowClass(const emscripten::val& value, ImGuiWindowClass& out) {
    out.ClassId = import_value<ImGuiID>(value["ClassId"]);
    out.TabItemFlagsOverrideSet = import_value<ImGuiTabItemFlags>(value["TabItemFlagsOverrideSet"]);
    out.DockNodeFlagsOverrideSet = import_value<ImGuiDockNodeFlags>(value["DockNodeFlagsOverrideSet"]);
    out.DockingAlwaysTabBar = import_value<bool>(value["DockingAlwaysTabBar"]);
    out.DockingAllowUnclassed = import_value<bool>(value["DockingAllowUnclassed"]);
    return out;
}

ImGuiWindowClass import_ImGuiWindowClass(const emscripten::val& value) {
    ImGuiWindowClass out; import_ImGuiWindowClass(value, out); return out;
}

EMSCRIPTEN_BINDINGS(ImGuiMultiSelectIO) {
    emscripten::class_<ImGuiMultiSelectIO>("ImGuiMultiSelectIO")
        .function("_getRequests", FUNCTION(emscripten::val, (const ImGuiMultiSelectIO& that), {
            emscripten::val arr = emscripten::val::array();
            for (int i = 0; i < that.Requests.Size; i++) {
                arr.call<void>("push", export_ImGuiSelectionRequest(that.Requests[i]));
            }
            return arr;
        }))
        .property("RangeSrcItem", FUNCTION(double, (const ImGuiMultiSelectIO& that), { return (double)that.RangeSrcItem; }),
            FUNCTION(void, (ImGuiMultiSelectIO& that, double v), { that.RangeSrcItem = (ImGuiSelectionUserData)v; }))
        .property("NavIdItem", FUNCTION(double, (const ImGuiMultiSelectIO& that), { return (double)that.NavIdItem; }),
            FUNCTION(void, (ImGuiMultiSelectIO& that, double v), { that.NavIdItem = (ImGuiSelectionUserData)v; }))
        CLASS_MEMBER(ImGuiMultiSelectIO, NavIdSelected)
        CLASS_MEMBER(ImGuiMultiSelectIO, RangeSrcReset)
        CLASS_MEMBER(ImGuiMultiSelectIO, ItemsCount)
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiSelectionBasicStorage) {
    emscripten::class_<ImGuiSelectionBasicStorage>("ImGuiSelectionBasicStorage")
        .constructor()
        CLASS_MEMBER(ImGuiSelectionBasicStorage, Size)
        CLASS_MEMBER(ImGuiSelectionBasicStorage, PreserveOrder)
        .function("ApplyRequests", FUNCTION(void, (ImGuiSelectionBasicStorage& that, ImGuiMultiSelectIO* ms_io), {
            that.ApplyRequests(ms_io);
        }), emscripten::allow_raw_pointers())
        .function("Contains", FUNCTION(bool, (const ImGuiSelectionBasicStorage& that, ImGuiID id), {
            return that.Contains(id);
        }))
        .function("Clear", FUNCTION(void, (ImGuiSelectionBasicStorage& that), { that.Clear(); }))
        .function("Swap", FUNCTION(void, (ImGuiSelectionBasicStorage& that, ImGuiSelectionBasicStorage& r), { that.Swap(r); }))
        .function("SetItemSelected", FUNCTION(void, (ImGuiSelectionBasicStorage& that, ImGuiID id, bool selected), {
            that.SetItemSelected(id, selected);
        }))
        .function("GetNextSelectedItem", FUNCTION(emscripten::val, (ImGuiSelectionBasicStorage& that, emscripten::val opaque_it_val), {
            void* opaque_it = opaque_it_val.isNull() ? NULL : (void*)(intptr_t)opaque_it_val.as<int>();
            ImGuiID out_id = 0;
            bool has_next = that.GetNextSelectedItem(&opaque_it, &out_id);
            emscripten::val result = emscripten::val::object();
            result.set("done", emscripten::val(!has_next));
            result.set("id", emscripten::val(out_id));
            result.set("it", opaque_it == NULL ? emscripten::val::null() : emscripten::val((int)(intptr_t)opaque_it));
            return result;
        }))
        .function("GetStorageIdFromIndex", FUNCTION(ImGuiID, (ImGuiSelectionBasicStorage& that, int idx), {
            return that.GetStorageIdFromIndex(idx);
        }))
        .function("SetAdapterIndexToStorageId", FUNCTION(void, (ImGuiSelectionBasicStorage& that, emscripten::val callback), {
            // Default adapter: index == storage id
            that.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx) -> ImGuiID { return (ImGuiID)idx; };
        }))
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiViewport) {
    emscripten::class_<ImGuiViewport>("ImGuiViewport")
        .constructor()
        // ImGuiID             ID;                     // Unique identifier for the viewport
        CLASS_MEMBER(ImGuiViewport, ID)
        // ImGuiViewportFlags  Flags;                  // See ImGuiViewportFlags_
        CLASS_MEMBER(ImGuiViewport, Flags)
        // ImVec2              Pos;                    // Main Area: Position of the viewport (the imgui coordinates are the same as OS desktop/native coordinates)
        CLASS_MEMBER(ImGuiViewport, Pos)
        // ImVec2              Size;                   // Main Area: Size of the viewport.
        CLASS_MEMBER(ImGuiViewport, Size)
        // ImVec2              WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
        CLASS_MEMBER(ImGuiViewport, WorkPos)
        // ImVec2              WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)
        CLASS_MEMBER(ImGuiViewport, WorkSize)
        // float               DpiScale;               // 1.0f = 96 DPI = No extra scale.
        CLASS_MEMBER(ImGuiViewport, DpiScale)
        // ImDrawData*         DrawData;               // The ImDrawData corresponding to this viewport. Valid after Render() and until the next call to NewFrame().
        // CLASS_MEMBER(ImGuiViewport, ID)
        // ImVec2 GetCenter() const
        .function("GetCenter", FUNCTION(emscripten::val, (const ImGuiViewport& that, emscripten::val out), {
            return export_ImVec2(that.GetCenter(), out);
        }))
        // ImVec2 GetWorkCenter() const
        .function("GetWorkCenter", FUNCTION(emscripten::val, (const ImGuiViewport& that, emscripten::val out), {
            return export_ImVec2(that.GetWorkCenter(), out);
        }))
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiPlatformMonitor) {
    emscripten::class_<ImGuiPlatformMonitor>("ImGuiPlatformMonitor")
        .constructor()
        CLASS_MEMBER(ImGuiPlatformMonitor, MainPos)
        CLASS_MEMBER(ImGuiPlatformMonitor, MainSize)
        CLASS_MEMBER(ImGuiPlatformMonitor, WorkPos)
        CLASS_MEMBER(ImGuiPlatformMonitor, WorkSize)
        CLASS_MEMBER(ImGuiPlatformMonitor, DpiScale)
        CLASS_MEMBER_GET_SET(ImGuiPlatformMonitor, PlatformHandle,
            { return that.PlatformHandle == NULL ? emscripten::val::null() : emscripten::val((intptr_t)that.PlatformHandle); },
            { that.PlatformHandle = value.isNull() ? NULL : (void*)(intptr_t)value.as<intptr_t>(); }
        )
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiPlatformIO) {
    emscripten::class_<ImGuiPlatformIO>("ImGuiPlatformIO")
        .function("IterateMonitors", FUNCTION(void, (ImGuiPlatformIO* that, emscripten::val callback), {
            for (int n = 0; n < that->Monitors.Size; n++) {
                callback(export_ImGuiPlatformMonitor(that->Monitors[n]));
            }
        }), emscripten::allow_raw_pointers())
        .function("IterateViewports", FUNCTION(void, (ImGuiPlatformIO* that, emscripten::val callback), {
            for (int n = 0; n < that->Viewports.Size; n++) {
                callback(emscripten::val(that->Viewports[n]));
            }
        }), emscripten::allow_raw_pointers())
        CLASS_MEMBER_GET_SET(ImGuiPlatformIO, Platform_LocaleDecimalPoint,
            { return emscripten::val((int)that.Platform_LocaleDecimalPoint); },
            { that.Platform_LocaleDecimalPoint = (ImWchar)value.as<int>(); }
        )
        CLASS_MEMBER(ImGuiPlatformIO, Renderer_TextureMaxWidth)
        CLASS_MEMBER(ImGuiPlatformIO, Renderer_TextureMaxHeight)
        CLASS_MEMBER_GET(ImGuiPlatformIO, Renderer_RenderState,
            { return that.Renderer_RenderState == NULL ? emscripten::val::null() : emscripten::val((intptr_t)that.Renderer_RenderState); }
        )
        .function("ClearPlatformHandlers", FUNCTION(void, (ImGuiPlatformIO& that), { that.ClearPlatformHandlers(); }))
        .function("ClearRendererHandlers", FUNCTION(void, (ImGuiPlatformIO& that), { that.ClearRendererHandlers(); }))
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiTableColumnSortSpecs) {
    emscripten::class_<ImGuiTableColumnSortSpecs>("ImGuiTableSortColumnSpecs")
        // ImGuiID                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
        CLASS_MEMBER(ImGuiTableColumnSortSpecs, ColumnUserID)
        // ImS16                       ColumnIndex;        // Index of the column
        CLASS_MEMBER(ImGuiTableColumnSortSpecs, ColumnIndex)
        // ImS16                       SortOrder;          // Index within parent ImGuiTableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
        CLASS_MEMBER(ImGuiTableColumnSortSpecs, SortOrder)
        // ImGuiSortDirection          SortDirection : 8;  // ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending (you can use this or SortSign, whichever is more convenient for your sort function)
        CLASS_MEMBER_GET(ImGuiTableColumnSortSpecs, SortDirection,
            { return emscripten::val(that.SortDirection); })
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiTableSortSpecs) {
    emscripten::class_<ImGuiTableSortSpecs>("ImGuiTableSortSpecs")
        // const ImGuiTableColumnSortSpecs* Specs;     // Pointer to sort spec array.
        // CLASS_MEMBER_GET_RAW_POINTER(ImGuiTableSortSpecs, Specs)
      .function("GetSpec", FUNCTION(emscripten::val, (ImGuiTableSortSpecs& that, const int idx), {
            const ImGuiTableColumnSortSpecs* spec = &that.Specs[idx];
            return emscripten::val(spec);
        }), emscripten::allow_raw_pointers())
        // int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when ImGuiTableFlags_SortMulti is enabled. May be == 0 when ImGuiTableFlags_SortTristate is enabled.
        CLASS_MEMBER(ImGuiTableSortSpecs, SpecsCount)
        // bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.
        CLASS_MEMBER(ImGuiTableSortSpecs, SpecsDirty)

        // ImGuiTableSortSpecs()       { memset(this, 0, sizeof(*this)); }
    ;
}

EMSCRIPTEN_BINDINGS(ImDrawCmd) {
    emscripten::class_<ImDrawCmd>("ImDrawCmd")
        CLASS_MEMBER(ImDrawCmd, ElemCount)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImDrawCmd, ClipRect)
        CLASS_MEMBER_GET(ImDrawCmd, TextureId, { return emscripten::val((int)(intptr_t) that.GetTexID()); })
        CLASS_MEMBER(ImDrawCmd, VtxOffset)
        CLASS_MEMBER(ImDrawCmd, IdxOffset)
    ;
}

EMSCRIPTEN_BINDINGS(ImDrawList) {
    emscripten::class_<ImDrawList>("ImDrawList")
        .function("IterateDrawCmds", FUNCTION(void, (const ImDrawList* that, emscripten::val callback), {
            unsigned int ElemStart = 0;
            for (int i = 0; i < that->CmdBuffer.Size; i++) {
                const ImDrawCmd* pcmd = &that->CmdBuffer[i];
                callback(emscripten::val(pcmd), emscripten::val(ElemStart));
                ElemStart += pcmd->ElemCount;
            }
        }), emscripten::allow_raw_pointers())

        // This is what you have to render
        // ImVector<ImDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
        // ImVector<ImDrawIdx>     IdxBuffer;          // Index buffer. Each command consume ImDrawCmd::ElemCount of those
        CLASS_MEMBER_GET(ImDrawList, IdxBuffer, {
            return emscripten::val(emscripten::typed_memory_view((size_t)(that.IdxBuffer.size() * sizeof(ImDrawIdx)), (char *) &that.IdxBuffer.front()));
        })
        CLASS_MEMBER_GET(ImDrawList, VtxBuffer, {
            return emscripten::val(emscripten::typed_memory_view((size_t)(that.VtxBuffer.size() * sizeof(ImDrawVert)), (char *) &that.VtxBuffer.front()));
        })
        // ImDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.
        CLASS_MEMBER(ImDrawList, Flags)

        // [Internal, used while building lists]
        // const ImDrawListSharedData* _Data;          // Pointer to shared draw data (you can use ImGui::GetDrawListSharedData() to get the one from current ImGui context)
        // const char*             _OwnerName;         // Pointer to owner window's name for debugging
        // unsigned int            _VtxCurrentIdx;     // [Internal] == VtxBuffer.Size
        // ImDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
        // ImDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
        // ImVector<ImVec4>        _ClipRectStack;     // [Internal]
        // ImVector<ImTextureID>   _TextureIdStack;    // [Internal]
        // ImVector<ImVec2>        _Path;              // [Internal] current path building
        // int                     _ChannelsCurrent;   // [Internal] current channel number (0)
        // int                     _ChannelsCount;     // [Internal] number of active channels (1+)
        // ImVector<ImDrawChannel> _Channels;          // [Internal] draw channels for columns API (not resized down so _ChannelsCount may be smaller than _Channels.Size)

        // ImDrawList(const ImDrawListSharedData* shared_data) { _Data = shared_data; _OwnerName = NULL; Clear(); }
        // ~ImDrawList() { ClearFreeMemory(); }
        // IMGUI_API void  PushClipRect(ImVec2 clip_rect_min, ImVec2 clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
        .function("PushClipRect", FUNCTION(void, (ImDrawList& that, emscripten::val clip_rect_min, emscripten::val clip_rect_max, bool intersect_with_current_clip_rect), {
            that.PushClipRect(import_ImVec2(clip_rect_min), import_ImVec2(clip_rect_max), intersect_with_current_clip_rect);
        }))
        // IMGUI_API void  PushClipRectFullScreen();
        CLASS_METHOD(ImDrawList, PushClipRectFullScreen)
        // IMGUI_API void  PopClipRect();
        CLASS_METHOD(ImDrawList, PopClipRect)
        // IMGUI_API void  PushTexture(ImTextureRef tex_ref);
        .function("PushTextureID", FUNCTION(void, (ImDrawList& that, emscripten::val texture_id), {
            that.PushTexture(ImTextureRef((ImTextureID)(intptr_t) texture_id.as<int>()));
        }))
        // IMGUI_API void  PopTexture();
        .function("PopTextureID", FUNCTION(void, (ImDrawList& that), {
            that.PopTexture();
        }))
        // inline ImVec2   GetClipRectMin() const { const ImVec4& cr = _ClipRectStack.back(); return ImVec2(cr.x, cr.y); }
        .function("GetClipRectMin", FUNCTION(emscripten::val, (ImDrawList& that, emscripten::val out), {
            return export_ImVec2(that.GetClipRectMin(), out);
        }))
        // inline ImVec2   GetClipRectMax() const { const ImVec4& cr = _ClipRectStack.back(); return ImVec2(cr.z, cr.w); }
        .function("GetClipRectMax", FUNCTION(emscripten::val, (ImDrawList& that, emscripten::val out), {
            return export_ImVec2(that.GetClipRectMax(), out);
        }))

        // Primitives
        // IMGUI_API void  AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness = 1.0f);
        .function("AddLine", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, ImU32 col, float thickness), {
            that.AddLine(import_ImVec2(a), import_ImVec2(b), col, thickness);
        }))
        // IMGUI_API void  AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding = 0.0f, int rounding_corners_flags = ImDrawCornerFlags_All, float thickness = 1.0f);   // a: upper-left, b: lower-right, rounding_corners_flags: 4-bits corresponding to which corner to round
        .function("AddRect", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, ImU32 col, float rounding, int rounding_corners_flags, float thickness), {
            that.AddRect(import_ImVec2(a), import_ImVec2(b), col, rounding, rounding_corners_flags, thickness);
        }))
        // IMGUI_API void  AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding = 0.0f, int rounding_corners_flags = ImDrawCornerFlags_All);                     // a: upper-left, b: lower-right
        .function("AddRectFilled", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, ImU32 col, float rounding, int rounding_corners_flags), {
            that.AddRectFilled(import_ImVec2(a), import_ImVec2(b), col, rounding, rounding_corners_flags);
        }))
        // IMGUI_API void  AddRectFilledMultiColor(const ImVec2& a, const ImVec2& b, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
        .function("AddRectFilledMultiColor", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left), {
            that.AddRectFilledMultiColor(import_ImVec2(a), import_ImVec2(b), col_upr_left, col_upr_right, col_bot_right, col_bot_left);
        }))
        // IMGUI_API void  AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness = 1.0f);
        .function("AddQuad", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, emscripten::val c, emscripten::val d, ImU32 col, float thickness), {
            that.AddQuad(import_ImVec2(a), import_ImVec2(b), import_ImVec2(c), import_ImVec2(d), col, thickness);
        }))
        // IMGUI_API void  AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col);
        .function("AddQuadFilled", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, emscripten::val c, emscripten::val d, ImU32 col), {
            that.AddQuadFilled(import_ImVec2(a), import_ImVec2(b), import_ImVec2(c), import_ImVec2(d), col);
        }))
        // IMGUI_API void  AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness = 1.0f);
        .function("AddTriangle", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, emscripten::val c, ImU32 col, float thickness), {
            that.AddTriangle(import_ImVec2(a), import_ImVec2(b), import_ImVec2(c), col, thickness);
        }))
        // IMGUI_API void  AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col);
        .function("AddTriangleFilled", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, emscripten::val c, ImU32 col), {
            that.AddTriangleFilled(import_ImVec2(a), import_ImVec2(b), import_ImVec2(c), col);
        }))
        // IMGUI_API void  AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments = 12, float thickness = 1.0f);
        .function("AddCircle", FUNCTION(void, (ImDrawList& that, emscripten::val centre, float radius, ImU32 col, int num_segments, float thickness), {
            that.AddCircle(import_ImVec2(centre), radius, col, num_segments, thickness);
        }))
        // IMGUI_API void  AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments = 12);
        .function("AddCircleFilled", FUNCTION(void, (ImDrawList& that, emscripten::val centre, float radius, ImU32 col, int num_segments), {
            that.AddCircleFilled(import_ImVec2(centre), radius, col, num_segments);
        }))
        // IMGUI_API void  AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness = 1.0f);
        .function("AddNgon", FUNCTION(void, (ImDrawList& that, emscripten::val centre, float radius, ImU32 col, int num_segments, float thickness), {
            that.AddNgon(import_ImVec2(centre), radius, col, num_segments, thickness);
        }))
        // IMGUI_API void  AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments);
        .function("AddNgonFilled", FUNCTION(void, (ImDrawList& that, emscripten::val centre, float radius, ImU32 col, int num_segments), {
            that.AddNgonFilled(import_ImVec2(centre), radius, col, num_segments);
        }))
        // IMGUI_API void  AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL);
        .function("AddText_A", FUNCTION(void, (ImDrawList& that, emscripten::val pos, ImU32 col, std::string text_begin), {
            that.AddText(import_ImVec2(pos), col, text_begin.c_str(), NULL);
        }))
        // IMGUI_API void  AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
        .function("AddText_B", FUNCTION(void, (ImDrawList& that, emscripten::val font, float font_size, emscripten::val pos, ImU32 col, std::string text_begin, float wrap_width, emscripten::val cpu_fine_clip_rect), {
            ImFont* _font = font.as<ImFont*>(emscripten::allow_raw_pointers());
            that.AddText(_font, font_size, import_ImVec2(pos), col, text_begin.c_str(), NULL, wrap_width, import_maybe_null_value<ImVec4>(cpu_fine_clip_rect));
        }))
        // IMGUI_API void  AddImage(ImTextureRef tex_ref, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min = ImVec2(0,0), const ImVec2& uv_max = ImVec2(1,1), ImU32 col = IM_COL32_WHITE);
        .function("AddImage", FUNCTION(void, (ImDrawList& that, emscripten::val user_texture_id, emscripten::val a, emscripten::val b, emscripten::val uv_a, emscripten::val uv_b, ImU32 col), {
            that.AddImage(ImTextureRef((ImTextureID)(intptr_t) user_texture_id.as<int>()), import_ImVec2(a), import_ImVec2(b), import_ImVec2(uv_a), import_ImVec2(uv_b), col);
        }))
        // IMGUI_API void  AddImageQuad(ImTextureRef tex_ref, ...);
        .function("AddImageQuad", FUNCTION(void, (ImDrawList& that, emscripten::val user_texture_id, emscripten::val a, emscripten::val b, emscripten::val c, emscripten::val d, emscripten::val uv_a, emscripten::val uv_b, emscripten::val uv_c, emscripten::val uv_d, ImU32 col), {
            that.AddImageQuad(ImTextureRef((ImTextureID)(intptr_t) user_texture_id.as<int>()), import_ImVec2(a), import_ImVec2(b), import_ImVec2(c), import_ImVec2(d), import_ImVec2(uv_a), import_ImVec2(uv_b), import_ImVec2(uv_c), import_ImVec2(uv_d), col);
        }))
        // IMGUI_API void  AddImageRounded(ImTextureRef tex_ref, ...);
        .function("AddImageRounded", FUNCTION(void, (ImDrawList& that, emscripten::val user_texture_id, emscripten::val a, emscripten::val b, emscripten::val uv_a, emscripten::val uv_b, ImU32 col, float rounding, ImDrawFlags flags), {
            that.AddImageRounded(ImTextureRef((ImTextureID)(intptr_t) user_texture_id.as<int>()), import_ImVec2(a), import_ImVec2(b), import_ImVec2(uv_a), import_ImVec2(uv_b), col, rounding, flags);
        }))
        // IMGUI_API void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
        .function("AddPolyline", FUNCTION(void, (ImDrawList& that, emscripten::val points, const int num_points, ImU32 col, ImDrawFlags flags, float thickness), {
            ImVec2 _points[num_points];
            for (int i = 0; i < num_points; ++i) {
                _points[i] = import_ImVec2(points[i]);
            }
            that.AddPolyline(_points, num_points, col, flags, thickness);
        }))
        // IMGUI_API void  AddConvexPolyFilled(const ImVec2* points, const int num_points, ImU32 col);
        .function("AddConvexPolyFilled", FUNCTION(void, (ImDrawList& that, emscripten::val points, const int num_points, ImU32 col), {
            ImVec2 _points[num_points];
            for (int i = 0; i < num_points; ++i) {
                _points[i] = import_ImVec2(points[i]);
            }
            that.AddConvexPolyFilled(_points, num_points, col);
        }))
        // IMGUI_API void  AddConcavePolyFilled(const ImVec2* points, int num_points, ImU32 col);
        .function("AddConcavePolyFilled", FUNCTION(void, (ImDrawList& that, emscripten::val points, const int num_points, ImU32 col), {
            ImVec2 _points[num_points];
            for (int i = 0; i < num_points; ++i) {
                _points[i] = import_ImVec2(points[i]);
            }
            that.AddConcavePolyFilled(_points, num_points, col);
        }))
        // IMGUI_API void  AddEllipse(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
        .function("AddEllipse", FUNCTION(void, (ImDrawList& that, emscripten::val center, emscripten::val radius, ImU32 col, float rot, int num_segments, float thickness), {
            that.AddEllipse(import_ImVec2(center), import_ImVec2(radius), col, rot, num_segments, thickness);
        }))
        // IMGUI_API void  AddEllipseFilled(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot = 0.0f, int num_segments = 0);
        .function("AddEllipseFilled", FUNCTION(void, (ImDrawList& that, emscripten::val center, emscripten::val radius, ImU32 col, float rot, int num_segments), {
            that.AddEllipseFilled(import_ImVec2(center), import_ImVec2(radius), col, rot, num_segments);
        }))
        // IMGUI_API void  AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
        .function("AddBezierCubic", FUNCTION(void, (ImDrawList& that, emscripten::val p1, emscripten::val p2, emscripten::val p3, emscripten::val p4, ImU32 col, float thickness, int num_segments), {
            that.AddBezierCubic(import_ImVec2(p1), import_ImVec2(p2), import_ImVec2(p3), import_ImVec2(p4), col, thickness, num_segments);
        }))
        .function("AddBezierQuadratic", FUNCTION(void, (ImDrawList& that, emscripten::val p1, emscripten::val p2, emscripten::val p3, ImU32 col, float thickness, int num_segments), {
            that.AddBezierQuadratic(import_ImVec2(p1), import_ImVec2(p2), import_ImVec2(p3), col, thickness, num_segments);
        }))

        // Stateful path API, add points then finish with PathFill() or PathStroke()
        // inline    void  PathClear()                                                 { _Path.resize(0); }
        CLASS_METHOD(ImDrawList, PathClear)
        // inline    void  PathLineTo(const ImVec2& pos)                               { _Path.push_back(pos); }
        .function("PathLineTo", FUNCTION(void, (ImDrawList& that, emscripten::val pos), {
            that.PathLineTo(import_ImVec2(pos));
        }))
        // inline    void  PathLineToMergeDuplicate(const ImVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path[_Path.Size-1], &pos, 8) != 0) _Path.push_back(pos); }
        .function("PathLineToMergeDuplicate", FUNCTION(void, (ImDrawList& that, emscripten::val pos), {
            that.PathLineToMergeDuplicate(import_ImVec2(pos));
        }))
        // inline    void  PathFillConvex(ImU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); PathClear(); }
        .function("PathFillConvex", FUNCTION(void, (ImDrawList& that, ImU32 col), {
            that.PathFillConvex(col);
        }))
        // inline    void  PathFillConcave(ImU32 col)                                 { AddConcavePolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
        .function("PathFillConcave", FUNCTION(void, (ImDrawList& that, ImU32 col), {
            that.PathFillConcave(col);
        }))
        // inline    void  PathStroke(ImU32 col, ImDrawFlags flags = 0, float thickness = 1.0f)
        .function("PathStroke", FUNCTION(void, (ImDrawList& that, ImU32 col, ImDrawFlags flags, float thickness), {
            that.PathStroke(col, flags, thickness);
        }))
        // IMGUI_API void  PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments = 10);
        .function("PathArcTo", FUNCTION(void, (ImDrawList& that, emscripten::val centre, float radius, float a_min, float a_max, int num_segments), {
            that.PathArcTo(import_ImVec2(centre), radius, a_min, a_max, num_segments);
        }))
        // IMGUI_API void  PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12);                                // Use precomputed angles for a 12 steps circle
        .function("PathArcToFast", FUNCTION(void, (ImDrawList& that, emscripten::val centre, float radius, int a_min_of_12, int a_max_of_12), {
            that.PathArcToFast(import_ImVec2(centre), radius, a_min_of_12, a_max_of_12);
        }))
        // IMGUI_API void  PathEllipticalArcTo(const ImVec2& center, const ImVec2& radius, float rot, float a_min, float a_max, int num_segments = 0);
        .function("PathEllipticalArcTo", FUNCTION(void, (ImDrawList& that, emscripten::val center, emscripten::val radius, float rot, float a_min, float a_max, int num_segments), {
            that.PathEllipticalArcTo(import_ImVec2(center), import_ImVec2(radius), rot, a_min, a_max, num_segments);
        }))
        // IMGUI_API void  PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments = 0);  // Cubic Bezier (4 control points)
        .function("PathBezierCubicCurveTo", FUNCTION(void, (ImDrawList& that, emscripten::val p2, emscripten::val p3, emscripten::val p4, int num_segments), {
            that.PathBezierCubicCurveTo(import_ImVec2(p2), import_ImVec2(p3), import_ImVec2(p4), num_segments);
        }))
        // IMGUI_API void  PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments = 0);                // Quadratic Bezier (3 control points)
        .function("PathBezierQuadraticCurveTo", FUNCTION(void, (ImDrawList& that, emscripten::val p2, emscripten::val p3, int num_segments), {
            that.PathBezierQuadraticCurveTo(import_ImVec2(p2), import_ImVec2(p3), num_segments);
        }))
        // IMGUI_API void  PathRect(const ImVec2& rect_min, const ImVec2& rect_max, float rounding = 0.0f, int rounding_corners_flags = ImDrawCornerFlags_All);
        .function("PathRect", FUNCTION(void, (ImDrawList& that, emscripten::val rect_min, emscripten::val rect_max, float rounding, int rounding_corners_flags), {
            that.PathRect(import_ImVec2(rect_min), import_ImVec2(rect_max), rounding, rounding_corners_flags);
        }))

        // Channels
        // - Use to simulate layers. By switching channels to can render out-of-order (e.g. submit foreground primitives before background primitives)
        // - Use to minimize draw calls (e.g. if going back-and-forth between multiple non-overlapping clipping rectangles, prefer to append into separate channels then merge at the end)
        // IMGUI_API void  ChannelsSplit(int channels_count);
        CLASS_METHOD(ImDrawList, ChannelsSplit)
        // IMGUI_API void  ChannelsMerge();
        CLASS_METHOD(ImDrawList, ChannelsMerge)
        // IMGUI_API void  ChannelsSetCurrent(int channel_index);
        CLASS_METHOD(ImDrawList, ChannelsSetCurrent)

        // Advanced
        // IMGUI_API void  AddCallback(ImDrawCallback callback, void* callback_data);  // Your rendering function must check for 'UserCallback' in ImDrawCmd and call the function instead of rendering triangles.
        .function("AddCallback", FUNCTION(void, (ImDrawList& that, emscripten::val callback, emscripten::val callback_data), {
            // TODO
        }))
        // IMGUI_API void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
        CLASS_METHOD(ImDrawList, AddDrawCmd)
        // IMGUI_API ImDrawList* CloneOutput() const;                                  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer.

        // Internal helpers
        // NB: all primitives needs to be reserved via PrimReserve() beforehand!
        // IMGUI_API void  PrimReserve(int idx_count, int vtx_count);
        CLASS_METHOD(ImDrawList, PrimReserve)
        // IMGUI_API void  PrimUnreserve(int idx_count, int vtx_count);
        CLASS_METHOD(ImDrawList, PrimUnreserve)
        // IMGUI_API void  PrimRect(const ImVec2& a, const ImVec2& b, ImU32 col);      // Axis aligned rectangle (composed of two triangles)
        .function("PrimRect", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, ImU32 col), {
            that.PrimRect(import_ImVec2(a), import_ImVec2(b), col);
        }))
        // IMGUI_API void  PrimRectUV(const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col);
        .function("PrimRectUV", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, emscripten::val uv_a, emscripten::val uv_b, ImU32 col), {
            that.PrimRectUV(import_ImVec2(a), import_ImVec2(b), import_ImVec2(uv_a), import_ImVec2(uv_b), col);
        }))
        // IMGUI_API void  PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col);
        .function("PrimQuadUV", FUNCTION(void, (ImDrawList& that, emscripten::val a, emscripten::val b, emscripten::val c, emscripten::val d, emscripten::val uv_a, emscripten::val uv_b, emscripten::val uv_c, emscripten::val uv_d, ImU32 col), {
            that.PrimQuadUV(import_ImVec2(a), import_ImVec2(b), import_ImVec2(c), import_ImVec2(d), import_ImVec2(uv_a), import_ImVec2(uv_b), import_ImVec2(uv_c), import_ImVec2(uv_d), col);
        }))
        // inline    void  PrimWriteVtx(const ImVec2& pos, const ImVec2& uv, ImU32 col){ _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
        .function("PrimWriteVtx", FUNCTION(void, (ImDrawList& that, emscripten::val pos, emscripten::val uv, ImU32 col), {
            that.PrimWriteVtx(import_ImVec2(pos), import_ImVec2(uv), col);
        }))
        // inline    void  PrimWriteIdx(ImDrawIdx idx)                                 { *_IdxWritePtr = idx; _IdxWritePtr++; }
        .function("PrimWriteIdx", FUNCTION(void, (ImDrawList& that, ImDrawIdx idx), {
            that.PrimWriteIdx(idx);
        }))
        // inline    void  PrimVtx(const ImVec2& pos, const ImVec2& uv, ImU32 col)     { PrimWriteIdx((ImDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); }
        .function("PrimVtx", FUNCTION(void, (ImDrawList& that, emscripten::val pos, emscripten::val uv, ImU32 col), {
            that.PrimVtx(import_ImVec2(pos), import_ImVec2(uv), col);
        }))
    ;
}

EMSCRIPTEN_BINDINGS(ImDrawData) {
    emscripten::class_<ImDrawData>("ImDrawData")
        .function("IterateDrawLists", FUNCTION(void, (const ImDrawData* that, emscripten::val callback), {
            for (int n = 0; n < that->CmdListsCount; n++) {
                const ImDrawList* cmd_list = that->CmdLists[n];
                callback(emscripten::val(cmd_list));
            }
        }), emscripten::allow_raw_pointers())

        // bool            Valid;                  // Only valid after Render() is called and before the next NewFrame() is called.
        CLASS_MEMBER(ImDrawData, Valid)
        // ImDrawList**    CmdLists;
        // int             CmdListsCount;
        CLASS_MEMBER(ImDrawData, CmdListsCount)
        // int             TotalIdxCount;          // For convenience, sum of all cmd_lists idx_buffer.Size
        CLASS_MEMBER(ImDrawData, TotalIdxCount)
        // int             TotalVtxCount;          // For convenience, sum of all cmd_lists vtx_buffer.Size
        CLASS_MEMBER(ImDrawData, TotalVtxCount)
        // ImVec2          DisplayPos;             // Upper-left position of the viewport to render (== upper-left of the orthogonal projection matrix to use)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImDrawData, DisplayPos)
        // ImVec2          DisplaySize;            // Size of the viewport to render (== io.DisplaySize for the main viewport) (DisplayPos + DisplaySize == lower-right of the orthogonal projection matrix to use)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImDrawData, DisplaySize)
        // ImVec2          FramebufferScale;       // Amount of pixels for each unit of DisplaySize. Based on io.DisplayFramebufferScale. Generally (1,1) on normal display, (2,2) on OSX with Retina display.
        CLASS_MEMBER_GET_RAW_REFERENCE(ImDrawData, FramebufferScale)

        // Functions
        // ImDrawData() { Valid = false; CmdLists = NULL; CmdListsCount = TotalVtxCount = TotalIdxCount = 0; }
        // IMGUI_API void DeIndexAllBuffers();               // For backward compatibility or convenience: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
        CLASS_METHOD(ImDrawData, DeIndexAllBuffers)
        // IMGUI_API void ScaleClipRects(const ImVec2& fb_scale);  // Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
        .function("ScaleClipRects", FUNCTION(void, (ImDrawData& that, emscripten::val fb_scale), {
            that.ScaleClipRects(import_ImVec2(fb_scale));
        }))
    ;
}

EMSCRIPTEN_BINDINGS(ImFontGlyph) {
    emscripten::class_<ImFontGlyph>("ImFontGlyph")
        // unsigned int    Codepoint : 31;     // 0x0000..0xFFFF
        // CLASS_MEMBER(ImFontGlyph, Codepoint)
        CLASS_MEMBER_GET(ImFontGlyph, Codepoint, { return emscripten::val(that.Codepoint); })
        // unsigned int    Visible : 1;        // Flag to allow early out when rendering
        // CLASS_MEMBER(ImFontGlyph, Visible)
        CLASS_MEMBER_GET(ImFontGlyph, Visible, { return emscripten::val(that.Visible != 0); })
        // float           AdvanceX;           // Distance to next character (= data from font + ImFontConfig::GlyphExtraSpacing.x baked in)
        CLASS_MEMBER(ImFontGlyph, AdvanceX)
        // float           X0, Y0, X1, Y1;     // Glyph corners
        CLASS_MEMBER(ImFontGlyph, X0)
        CLASS_MEMBER(ImFontGlyph, Y0)
        CLASS_MEMBER(ImFontGlyph, X1)
        CLASS_MEMBER(ImFontGlyph, Y1)
        // float           U0, V0, U1, V1;     // Texture coordinates
        CLASS_MEMBER(ImFontGlyph, U0)
        CLASS_MEMBER(ImFontGlyph, V0)
        CLASS_MEMBER(ImFontGlyph, U1)
        CLASS_MEMBER(ImFontGlyph, V1)
    ;
}

EMSCRIPTEN_BINDINGS(ImFontBaked) {
    emscripten::class_<ImFontBaked>("ImFontBaked")
        .function("IterateGlyphs", FUNCTION(void, (ImFontBaked* that, emscripten::val callback), {
            for (int n = 0; n < that->Glyphs.Size; n++) {
                callback(export_ImFontGlyph(that->Glyphs[n]));
            }
        }), emscripten::allow_raw_pointers())
        CLASS_MEMBER(ImFontBaked, FallbackAdvanceX)
        CLASS_MEMBER(ImFontBaked, Size)
        CLASS_MEMBER(ImFontBaked, RasterizerDensity)
        CLASS_MEMBER(ImFontBaked, FallbackGlyphIndex)
        CLASS_MEMBER(ImFontBaked, Ascent)
        CLASS_MEMBER(ImFontBaked, Descent)
        CLASS_MEMBER_GET_RAW_POINTER(ImFontBaked, OwnerFont)
        .function("FindGlyph", FUNCTION(emscripten::val, (ImFontBaked& that, ImWchar c), {
            ImFontGlyph* glyph = that.FindGlyph(c);
            return glyph == NULL ? emscripten::val::null() : emscripten::val(glyph);
        }), emscripten::allow_raw_pointers())
        .function("FindGlyphNoFallback", FUNCTION(emscripten::val, (ImFontBaked& that, ImWchar c), {
            ImFontGlyph* glyph = that.FindGlyphNoFallback(c);
            return glyph == NULL ? emscripten::val::null() : emscripten::val(glyph);
        }), emscripten::allow_raw_pointers())
        .function("GetCharAdvance", FUNCTION(float, (ImFontBaked& that, ImWchar c), { return that.GetCharAdvance(c); }))
        .function("IsGlyphLoaded", FUNCTION(bool, (ImFontBaked& that, ImWchar c), { return that.IsGlyphLoaded(c); }))
    ;
}

EMSCRIPTEN_BINDINGS(ImFontConfig) {
    emscripten::class_<ImFontConfig>("ImFontConfig")
        // char            Name[40];
        CLASS_MEMBER_GET_SET(ImFontConfig, Name,
            { return emscripten::val(std::string(that.Name)); },
            { strncpy(that.Name, value.as<std::string>().c_str(), sizeof(that.Name) - 1); }
        )
        // void*           FontData;
        CLASS_MEMBER_GET_SET(ImFontConfig, FontData,
            { TODO(); return emscripten::val::null(); },
            { TODO(); }
        )
        // int             FontDataSize;
        CLASS_MEMBER(ImFontConfig, FontDataSize)
        // bool            FontDataOwnedByAtlas;
        CLASS_MEMBER(ImFontConfig, FontDataOwnedByAtlas)
        // bool            MergeMode;
        CLASS_MEMBER(ImFontConfig, MergeMode)
        // bool            PixelSnapH;
        CLASS_MEMBER(ImFontConfig, PixelSnapH)
        // ImS8            OversampleH, OversampleV;
        CLASS_MEMBER_GET_SET(ImFontConfig, OversampleH,
            { return emscripten::val((int)that.OversampleH); },
            { that.OversampleH = (ImS8)value.as<int>(); }
        )
        CLASS_MEMBER_GET_SET(ImFontConfig, OversampleV,
            { return emscripten::val((int)that.OversampleV); },
            { that.OversampleV = (ImS8)value.as<int>(); }
        )
        // float           SizePixels;
        CLASS_MEMBER(ImFontConfig, SizePixels)
        // const ImWchar*  GlyphRanges;
        CLASS_MEMBER_GET(ImFontConfig, GlyphRanges, {
            return that.GlyphRanges == NULL ? emscripten::val::null() : emscripten::val((intptr_t) that.GlyphRanges);
        })
        // ImVec2          GlyphOffset;
        CLASS_MEMBER_GET_RAW_REFERENCE(ImFontConfig, GlyphOffset)
        // float           GlyphMinAdvanceX;
        CLASS_MEMBER(ImFontConfig, GlyphMinAdvanceX)
        // float           GlyphMaxAdvanceX;
        CLASS_MEMBER(ImFontConfig, GlyphMaxAdvanceX)
        // ImU32           FontNo;
        CLASS_MEMBER(ImFontConfig, FontNo)
        // unsigned int    FontLoaderFlags;
        CLASS_MEMBER(ImFontConfig, FontLoaderFlags)
        // float           RasterizerMultiply;
        CLASS_MEMBER(ImFontConfig, RasterizerMultiply)
        // float           RasterizerDensity;
        CLASS_MEMBER(ImFontConfig, RasterizerDensity)
        // ImWchar         EllipsisChar;
        CLASS_MEMBER(ImFontConfig, EllipsisChar)
        // ImFont*         DstFont;
        CLASS_MEMBER_GET_RAW_POINTER(ImFontConfig, DstFont)
    ;
}

EMSCRIPTEN_BINDINGS(ImFont) {
    emscripten::class_<ImFont>("ImFont")
        // float                       LegacySize;         // Font size passed to AddFont()
        CLASS_MEMBER(ImFont, LegacySize)
        // ImWchar                     FallbackChar;       // Character used if a glyph isn't found (U+FFFD, '?')
        CLASS_MEMBER(ImFont, FallbackChar)
        // ImWchar                     EllipsisChar;       // Character used for ellipsis rendering ('...')
        CLASS_MEMBER(ImFont, EllipsisChar)
        // ImGuiID                     FontId;
        CLASS_MEMBER(ImFont, FontId)

        // Iterate glyphs via ImFontBaked
        .function("IterateGlyphs", FUNCTION(void, (ImFont* that, emscripten::val callback), {
            ImFontBaked* baked = that->GetFontBaked(that->LegacySize);
            if (baked) {
                for (int n = 0; n < baked->Glyphs.Size; n++) {
                    callback(export_ImFontGlyph(baked->Glyphs[n]));
                }
            }
        }), emscripten::allow_raw_pointers())

        // Iterate sources (was ConfigData)
        .function("IterateConfigData", FUNCTION(void, (ImFont* that, emscripten::val callback), {
            for (int n = 0; n < that->Sources.Size; n++) {
                ImFontConfig* cfg = that->Sources[n];
                callback(emscripten::val(cfg));
            }
        }), emscripten::allow_raw_pointers())

        // Methods
        // IMGUI_API void              ClearOutputData();
        CLASS_METHOD(ImFont, ClearOutputData)
        // bool                        IsLoaded() const
        CLASS_METHOD(ImFont, IsLoaded)
        // const char*                 GetDebugName() const
        .function("GetDebugName", FUNCTION(std::string, (const ImFont& that), { return that.GetDebugName(); }))

        .function("GetFontBaked", FUNCTION(emscripten::val, (ImFont& that, float font_size, float density), {
            ImFontBaked* baked = that.GetFontBaked(font_size, density);
            return baked == NULL ? emscripten::val::null() : emscripten::val(baked);
        }), emscripten::allow_raw_pointers())

        // FindGlyph/FindGlyphNoFallback via ImFontBaked
        .function("FindGlyph", FUNCTION(emscripten::val, (ImFont& that, ImWchar c), {
            ImFontBaked* baked = that.GetFontBaked(that.LegacySize);
            if (!baked) return emscripten::val::null();
            ImFontGlyph* glyph = baked->FindGlyph(c);
            return glyph == NULL ? emscripten::val::null() : emscripten::val(glyph);
        }), emscripten::allow_raw_pointers())
        .function("FindGlyphNoFallback", FUNCTION(emscripten::val, (ImFont& that, ImWchar c), {
            ImFontBaked* baked = that.GetFontBaked(that.LegacySize);
            if (!baked) return emscripten::val::null();
            ImFontGlyph* glyph = baked->FindGlyphNoFallback(c);
            return glyph == NULL ? emscripten::val::null() : emscripten::val(glyph);
        }), emscripten::allow_raw_pointers())

        // GetCharAdvance via ImFontBaked
        .function("GetCharAdvance", FUNCTION(float, (ImFont& that, ImWchar c), {
            ImFontBaked* baked = that.GetFontBaked(that.LegacySize);
            return baked ? baked->GetCharAdvance(c) : 0.0f;
        }))

        // CalcTextSizeA
        .function("CalcTextSizeA", FUNCTION(emscripten::val, (ImFont& that, float size, float max_width, float wrap_width, std::string text_begin, emscripten::val remaining, emscripten::val out), {
            const char* _text_begin = text_begin.c_str();
            const char* _remaining = NULL;
            const ImVec2 text_size = that.CalcTextSizeA(size, max_width, wrap_width, _text_begin, NULL, &_remaining);
            if (!remaining.isNull()) {
                remaining.set(0, (int)(_remaining - _text_begin));
            }
            return export_ImVec2(text_size, out);
        }))

        // CalcWordWrapPosition (renamed from CalcWordWrapPositionA)
        .function("CalcWordWrapPositionA", FUNCTION(int, (ImFont& that, float size, std::string text, float wrap_width), {
            const char* _text = text.c_str();
            const char* pos = that.CalcWordWrapPosition(size, _text, NULL, wrap_width);
            return (int)(pos - _text);
        }))

        // RenderChar
        .function("RenderChar", FUNCTION(void, (ImFont& that, emscripten::val draw_list, float size, emscripten::val pos, ImU32 col, unsigned short c), {
            that.RenderChar(draw_list.as<ImDrawList*>(emscripten::allow_raw_pointers()), size, import_ImVec2(pos), col, c);
        }))

        // IsGlyphInFont (new)
        .function("IsGlyphInFont", FUNCTION(bool, (ImFont& that, ImWchar c), {
            return that.IsGlyphInFont(c);
        }))

        .function("IsGlyphLoaded", FUNCTION(bool, (ImFont& that, ImWchar c), {
            ImFontBaked* baked = that.GetFontBaked(that.LegacySize);
            return baked ? baked->IsGlyphLoaded(c) : false;
        }))

        // AddRemapChar
        .function("AddRemapChar", FUNCTION(void, (ImFont& that, ImWchar from_codepoint, ImWchar to_codepoint), {
            that.AddRemapChar(from_codepoint, to_codepoint);
        }))

        .function("RenderText", FUNCTION(void, (ImFont& that, emscripten::val draw_list, float size, emscripten::val pos, ImU32 col, emscripten::val clip_rect, std::string text_begin, float wrap_width, ImDrawFlags flags), {
            const char* _text_begin = text_begin.c_str();
            that.RenderText(draw_list.as<ImDrawList*>(emscripten::allow_raw_pointers()), size, import_ImVec2(pos), col, import_ImVec4(clip_rect), _text_begin, NULL, wrap_width, flags);
        }))
    ;
}

ImFontConfig import_ImFontConfig(emscripten::val value) {
    ImFontConfig font_cfg;
    // char            Name[40];
    strncpy(font_cfg.Name, value["Name"].as<std::string>().c_str(), sizeof(font_cfg.Name) - 1);
    // void*           FontData;
    const emscripten::val FontData = value["FontData"];
    if (FontData.isNull()) {
        font_cfg.FontData = NULL;
        font_cfg.FontDataSize = 0;
    } else {
        const size_t byteLength = FontData["byteLength"].as<size_t>();
        void* data_copy = ImGui::MemAlloc(byteLength);
        emscripten::val(emscripten::typed_memory_view<unsigned char>(byteLength, (unsigned char*)data_copy)).call<void>("set", FontData);
        font_cfg.FontData = data_copy;
        font_cfg.FontDataSize = (int)byteLength;
    }
    // bool            FontDataOwnedByAtlas;
    font_cfg.FontDataOwnedByAtlas = value["FontDataOwnedByAtlas"].as<bool>();
    // bool            MergeMode;
    font_cfg.MergeMode = value["MergeMode"].as<bool>();
    // bool            PixelSnapH;
    font_cfg.PixelSnapH = value["PixelSnapH"].as<bool>();
    // ImS8            OversampleH, OversampleV;
    font_cfg.OversampleH = (ImS8)value["OversampleH"].as<int>();
    font_cfg.OversampleV = (ImS8)value["OversampleV"].as<int>();
    // float           SizePixels;
    font_cfg.SizePixels = import_value<float>(value["SizePixels"]);
    // const ImWchar*  GlyphRanges;
    const emscripten::val GlyphRanges = value["GlyphRanges"];
    font_cfg.GlyphRanges = GlyphRanges.isNull() ? NULL : (const ImWchar*) GlyphRanges.as<intptr_t>();
    // ImVec2          GlyphOffset;
    font_cfg.GlyphOffset = import_ImVec2(value["GlyphOffset"]);
    // float           GlyphMinAdvanceX;
    font_cfg.GlyphMinAdvanceX = import_value<float>(value["GlyphMinAdvanceX"]);
    // float           GlyphMaxAdvanceX;
    font_cfg.GlyphMaxAdvanceX = import_value<float>(value["GlyphMaxAdvanceX"]);
    // ImU32           FontNo;
    font_cfg.FontNo = value["FontNo"].as<unsigned int>();
    // unsigned int    FontLoaderFlags;
    font_cfg.FontLoaderFlags = value["FontLoaderFlags"].as<unsigned int>();
    // float           RasterizerMultiply;
    font_cfg.RasterizerMultiply = import_value<float>(value["RasterizerMultiply"]);
    return font_cfg;
}

EMSCRIPTEN_BINDINGS(ImFontAtlas) {
    emscripten::class_<ImFontAtlas>("ImFontAtlas")
        // IMGUI_API ImFontAtlas();
        // IMGUI_API ~ImFontAtlas();
        // IMGUI_API ImFont*           AddFont(const ImFontConfig* font_cfg);
        .function("AddFont", FUNCTION(emscripten::val, (ImFontAtlas& that, emscripten::val font_cfg), {
            ImFontConfig _font_cfg = import_ImFontConfig(font_cfg);
            ImFont* font = that.AddFont(&_font_cfg);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API ImFont*           AddFontDefault(const ImFontConfig* font_cfg = NULL);
        .function("AddFontDefault", FUNCTION(emscripten::val, (ImFontAtlas& that, emscripten::val font_cfg), {
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            ImFont* font = that.AddFontDefault(font_cfg.isNull() ? NULL : &_font_cfg);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        .function("AddFontDefaultVector", FUNCTION(emscripten::val, (ImFontAtlas& that, emscripten::val font_cfg), {
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            ImFont* font = that.AddFontDefaultVector(font_cfg.isNull() ? NULL : &_font_cfg);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        .function("AddFontDefaultBitmap", FUNCTION(emscripten::val, (ImFontAtlas& that, emscripten::val font_cfg), {
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            ImFont* font = that.AddFontDefaultBitmap(font_cfg.isNull() ? NULL : &_font_cfg);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API ImFont*           AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);
        .function("AddFontFromFileTTF", FUNCTION(emscripten::val, (ImFontAtlas& that, std::string filename, float size_pixels, emscripten::val font_cfg, emscripten::val glyph_ranges), {
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            const ImWchar* _glyph_ranges = glyph_ranges.isNull() ? NULL : (const ImWchar*)glyph_ranges.as<intptr_t>();
            ImFont* font = that.AddFontFromFileTTF(filename.c_str(), size_pixels, font_cfg.isNull() ? NULL : &_font_cfg, _glyph_ranges);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API ImFont*           AddFontFromMemoryTTF(void* font_data, int font_size, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to ImFontAtlas! Will be deleted after Build(). Set font_cfg->FontDataOwnedByAtlas to false to keep ownership.
        .function("AddFontFromMemoryTTF", FUNCTION(emscripten::val, (ImFontAtlas& that, emscripten::val data, float size_pixels, emscripten::val font_cfg, emscripten::val glyph_ranges), {
            std::vector<unsigned char> _data;
            _data.resize(data["length"].as<size_t>());
            emscripten::val(emscripten::typed_memory_view<unsigned char>(_data.size(), _data.data())).call<void>("set", data);
            size_t _data_size = _data.size();
            void* _data_copy = ImGui::MemAlloc(_data_size);
            memcpy(_data_copy, _data.data(), _data_size);
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            ImWchar* _glyph_ranges = glyph_ranges.isNull() ? NULL : (ImWchar*) glyph_ranges.as<intptr_t>();
            if (!font_cfg.isNull()) {
                int i = 0;
                while (true) {
                  ImWchar ch = _glyph_ranges[i];
                  if (ch == 0) {
                      break;
                  }
                  ++i;
                }
            }
            ImFont* font = that.AddFontFromMemoryTTF(_data_copy, _data_size, size_pixels, font_cfg.isNull() ? NULL : &_font_cfg, _glyph_ranges);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API ImFont*           AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_size, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
        .function("AddFontFromMemoryCompressedTTF", FUNCTION(emscripten::val, (ImFontAtlas& that, emscripten::val data, float size_pixels, emscripten::val font_cfg, emscripten::val glyph_ranges), {
            std::vector<unsigned char> _data;
            _data.resize(data["length"].as<size_t>());
            emscripten::val(emscripten::typed_memory_view<unsigned char>(_data.size(), _data.data())).call<void>("set", data);
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            const ImWchar* _glyph_ranges = glyph_ranges.isNull() ? NULL : (const ImWchar*)glyph_ranges.as<intptr_t>();
            ImFont* font = that.AddFontFromMemoryCompressedTTF(_data.data(), (int)_data.size(), size_pixels, font_cfg.isNull() ? NULL : &_font_cfg, _glyph_ranges);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API ImFont*           AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
        .function("AddFontFromMemoryCompressedBase85TTF", FUNCTION(emscripten::val, (ImFontAtlas& that, std::string data, float size_pixels, emscripten::val font_cfg, emscripten::val glyph_ranges), {
            ImFontConfig _font_cfg = font_cfg.isNull() ? ImFontConfig() : import_ImFontConfig(font_cfg);
            const ImWchar* _glyph_ranges = glyph_ranges.isNull() ? NULL : (const ImWchar*)glyph_ranges.as<intptr_t>();
            ImFont* font = that.AddFontFromMemoryCompressedBase85TTF(data.c_str(), size_pixels, font_cfg.isNull() ? NULL : &_font_cfg, _glyph_ranges);
            return emscripten::val(font);
        }), emscripten::allow_raw_pointers())
        .function("RemoveFont", FUNCTION(void, (ImFontAtlas& that, emscripten::val font), {
            if (!font.isNull()) {
                that.RemoveFont(font.as<ImFont*>(emscripten::allow_raw_pointers()));
            }
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void              ClearTexData();             // Clear the CPU-side texture data. Saves RAM once the texture has been copied to graphics memory.
        CLASS_METHOD(ImFontAtlas, ClearTexData)
        // IMGUI_API void              ClearInputData();           // Clear the input TTF data (inc sizes, glyph ranges)
        CLASS_METHOD(ImFontAtlas, ClearInputData)
        // IMGUI_API void              ClearFonts();               // Clear the ImGui-side font data (glyphs storage, UV coordinates)
        CLASS_METHOD(ImFontAtlas, ClearFonts)
        // IMGUI_API void              Clear();                    // Clear all
        CLASS_METHOD(ImFontAtlas, Clear)
        // IMGUI_API void              CompactCache();
        CLASS_METHOD(ImFontAtlas, CompactCache)
        
        // Build atlas, retrieve pixel data.
        // User is in charge of copying the pixels into graphics memory (e.g. create a texture with your engine). Then store your texture handle with SetTexID().
        // RGBA32 format is provided for convenience and compatibility, but note that unless you use CustomRect to draw color data, the RGB pixels emitted from Fonts will all be white (~75% of waste). 
        // Pitch = Width * BytesPerPixels
        // IMGUI_API bool              Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
        CLASS_METHOD(ImFontAtlas, Build)
        // IMGUI_API bool              IsBuilt()                   { return Fonts.Size > 0 && (TexPixelsAlpha8 != NULL || TexPixelsRGBA32 != NULL); }
        CLASS_METHOD(ImFontAtlas, IsBuilt)
        // IMGUI_API void              GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL);  // 1 byte per-pixel
        .function("GetTexDataAsAlpha8", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            unsigned char* pixels = NULL;
            int width = -1;
            int height = -1;
            int bytes_per_pixel = -1;
            that.GetTexDataAsAlpha8(&pixels, &width, &height, &bytes_per_pixel);
            emscripten::val tex_data = emscripten::val::object();
            tex_data.set(emscripten::val("pixels"), emscripten::val(emscripten::typed_memory_view(width * height * bytes_per_pixel, pixels)));
            tex_data.set(emscripten::val("width"), emscripten::val(width));
            tex_data.set(emscripten::val("height"), emscripten::val(height));
            tex_data.set(emscripten::val("bytes_per_pixel"), emscripten::val(bytes_per_pixel));
            return tex_data;
        }))
        // IMGUI_API void              GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL);  // 4 bytes-per-pixel
        .function("GetTexDataAsRGBA32", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            unsigned char* pixels = NULL;
            int width = -1;
            int height = -1;
            int bytes_per_pixel = -1;
            that.GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);
            emscripten::val tex_data = emscripten::val::object();
            tex_data.set(emscripten::val("pixels"), emscripten::val(emscripten::typed_memory_view(width * height * bytes_per_pixel, pixels)));
            tex_data.set(emscripten::val("width"), emscripten::val(width));
            tex_data.set(emscripten::val("height"), emscripten::val(height));
            tex_data.set(emscripten::val("bytes_per_pixel"), emscripten::val(bytes_per_pixel));
            return tex_data;
        }))
        // void                        SetTexID(ImTextureID id)    { TexID = id; }

        //-------------------------------------------
        // Glyph Ranges
        //-------------------------------------------

        // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
        // NB: Make sure that your string are UTF-8 and NOT in your local code page. In C++11, you can create UTF-8 string literal using the u8"Hello world" syntax. See FAQ for details.
        // IMGUI_API const ImWchar*    GetGlyphRangesDefault();    // Basic Latin, Extended Latin
        .function("GetGlyphRangesDefault", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesDefault());
        }))
        .function("GetGlyphRangesGreek", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesGreek());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesKorean();     // Default + Korean characters
        .function("GetGlyphRangesKorean", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesKorean());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesJapanese();   // Default + Hiragana, Katakana, Half-Width, Selection of 1946 Ideographs
        .function("GetGlyphRangesJapanese", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesJapanese());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
        .function("GetGlyphRangesChineseFull", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesChineseFull());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
        .function("GetGlyphRangesChineseSimplifiedCommon", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesChineseSimplifiedCommon());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesCyrillic();   // Default + about 400 Cyrillic characters
        .function("GetGlyphRangesCyrillic", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesCyrillic());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesThai();       // Default + Thai characters
        .function("GetGlyphRangesThai", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesThai());
        }))
        // IMGUI_API const ImWchar*    GetGlyphRangesVietnamese();       // Default + Vietnamese characters
        .function("GetGlyphRangesVietnamese", FUNCTION(emscripten::val, (ImFontAtlas& that), {
            return emscripten::val((intptr_t) that.GetGlyphRangesVietnamese());
        }))

        // Helpers to build glyph ranges from text data. Feed your application strings/characters to it then call BuildRanges().
        // struct GlyphRangesBuilder
        // {
        //     ImVector<unsigned char> UsedChars;  // Store 1-bit per Unicode code point (0=unused, 1=used)
        //     GlyphRangesBuilder()                { UsedChars.resize(0x10000 / 8); memset(UsedChars.Data, 0, 0x10000 / 8); }
        //     bool           GetBit(int n)        { return (UsedChars[n >> 3] & (1 << (n & 7))) != 0; }
        //     void           SetBit(int n)        { UsedChars[n >> 3] |= 1 << (n & 7); }  // Set bit 'c' in the array
        //     void           AddChar(ImWchar c)   { SetBit(c); }                          // Add character
        //     IMGUI_API void AddText(const char* text, const char* text_end = NULL);      // Add string (each character of the UTF-8 string are added)
        //     IMGUI_API void AddRanges(const ImWchar* ranges);                            // Add ranges, e.g. builder.AddRanges(ImFontAtlas::GetGlyphRangesDefault) to force add all of ASCII/Latin+Ext
        //     IMGUI_API void BuildRanges(ImVector<ImWchar>* out_ranges);                  // Output new ranges
        // };

        //-------------------------------------------
        // Custom Rectangles/Glyphs API
        //-------------------------------------------

        // You can request arbitrary rectangles to be packed into the atlas, for your own purposes. After calling Build(), you can query the rectangle position and render your pixels.
        // You can also request your rectangles to be mapped as font glyph (given a font + Unicode point), so you can render e.g. custom colorful icons and use them as regular glyphs.
        // struct CustomRect
        // {
        //     unsigned int    ID;             // Input    // User ID. Use <0x10000 to map into a font glyph, >=0x10000 for other/internal/custom texture data.
        //     unsigned short  Width, Height;  // Input    // Desired rectangle dimension
        //     unsigned short  X, Y;           // Output   // Packed position in Atlas
        //     float           GlyphAdvanceX;  // Input    // For custom font glyphs only (ID<0x10000): glyph xadvance
        //     ImVec2          GlyphOffset;    // Input    // For custom font glyphs only (ID<0x10000): glyph display offset
        //     ImFont*         Font;           // Input    // For custom font glyphs only (ID<0x10000): target font
        //     CustomRect()            { ID = 0xFFFFFFFF; Width = Height = 0; X = Y = 0xFFFF; GlyphAdvanceX = 0.0f; GlyphOffset = ImVec2(0,0); Font = NULL; }
        //     bool IsPacked() const   { return X != 0xFFFF; }
        // };

        // IMGUI_API int       AddCustomRectRegular(unsigned int id, int width, int height);                                                                   // Id needs to be >= 0x10000. Id >= 0x80000000 are reserved for ImGui and ImDrawList
        // IMGUI_API int       AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset = ImVec2(0,0));   // Id needs to be < 0x10000 to register a rectangle to map into a specific font.
        .function("AddCustomRect", FUNCTION(ImFontAtlasRectId, (ImFontAtlas& that, int width, int height), {
            return that.AddCustomRect(width, height, NULL);
        }))
        .function("RemoveCustomRect", FUNCTION(void, (ImFontAtlas& that, ImFontAtlasRectId id), {
            that.RemoveCustomRect(id);
        }))
        .function("GetCustomRect", FUNCTION(emscripten::val, (const ImFontAtlas& that, ImFontAtlasRectId id), {
            ImFontAtlasRect rect;
            if (!that.GetCustomRect(id, &rect)) {
                return emscripten::val::null();
            }
            return export_ImFontAtlasRect(rect);
        }))
        .function("AddCustomRectFontGlyph", FUNCTION(ImFontAtlasRectId, (ImFontAtlas& that, emscripten::val font, ImWchar codepoint, int w, int h, float advance_x, emscripten::val offset), {
            ImFont* _font = font.isNull() ? NULL : font.as<ImFont*>(emscripten::allow_raw_pointers());
            return that.AddCustomRectFontGlyph(_font, codepoint, w, h, advance_x, import_ImVec2(offset));
        }), emscripten::allow_raw_pointers())
        .function("AddCustomRectFontGlyphForSize", FUNCTION(ImFontAtlasRectId, (ImFontAtlas& that, emscripten::val font, float font_size, ImWchar codepoint, int w, int h, float advance_x, emscripten::val offset), {
            ImFont* _font = font.isNull() ? NULL : font.as<ImFont*>(emscripten::allow_raw_pointers());
            return that.AddCustomRectFontGlyphForSize(_font, font_size, codepoint, w, h, advance_x, import_ImVec2(offset));
        }), emscripten::allow_raw_pointers())

        //-------------------------------------------
        // Members
        //-------------------------------------------

        // bool                        Locked;
        CLASS_MEMBER(ImFontAtlas, Locked)
        // ImFontAtlasFlags            Flags;
        CLASS_MEMBER(ImFontAtlas, Flags)
        // int                         TexMinWidth;
        CLASS_MEMBER(ImFontAtlas, TexMinWidth)
        // int                         TexMinHeight;
        CLASS_MEMBER(ImFontAtlas, TexMinHeight)
        // int                         TexMaxWidth;
        CLASS_MEMBER(ImFontAtlas, TexMaxWidth)
        // int                         TexMaxHeight;
        CLASS_MEMBER(ImFontAtlas, TexMaxHeight)
        // ImTextureRef TexRef / TexID (union for backward compatibility)
        CLASS_MEMBER_GET_SET(ImFontAtlas, TexID,
            { return emscripten::val((int)(intptr_t) that.TexRef.GetTexID()); },
            { that.SetTexID((ImTextureID)(intptr_t) value.as<int>()); }
        )
        // int                         TexGlyphPadding;
        CLASS_MEMBER(ImFontAtlas, TexGlyphPadding)
        // TexWidth/TexHeight via TexData
        CLASS_MEMBER_GET(ImFontAtlas, TexWidth,
            { return emscripten::val(that.TexData ? that.TexData->Width : 0); }
        )
        CLASS_MEMBER_GET(ImFontAtlas, TexHeight,
            { return emscripten::val(that.TexData ? that.TexData->Height : 0); }
        )
        // ImVec2                      TexUvScale;
        CLASS_MEMBER_GET_RAW_REFERENCE(ImFontAtlas, TexUvScale)
        // ImVec2                      TexUvWhitePixel;
        CLASS_MEMBER_GET_RAW_REFERENCE(ImFontAtlas, TexUvWhitePixel)
        // ImVector<ImFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling ImGui::NewFrame(), use ImGui::PushFont()/PopFont() to change the current font.
        .function("IterateFonts", FUNCTION(void, (ImFontAtlas* that, emscripten::val callback), {
            for (int n = 0; n < that->Fonts.Size; n++) {
                ImFont* font = that->Fonts.Data[n];
                callback(emscripten::val(font));
            }
        }), emscripten::allow_raw_pointers())
        // ImVector<CustomRect>        CustomRects;        // Rectangles for packing custom texture data into the atlas.
        // ImVector<ImFontConfig>      ConfigData;         // Internal data
        // int                         CustomRectIds[1];   // Identifiers of custom texture rectangle used by ImFontAtlas/ImDrawList
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiIO) {
    emscripten::class_<ImGuiIO>("ImGuiIO")
        //------------------------------------------------------------------
        // Settings (fill once)                 // Default value:
        //------------------------------------------------------------------

        // ImGuiConfigFlags ConfigFlags;           // = 0                  // See ImGuiConfigFlags_. Gamepad/keyboard navigation options.
        CLASS_MEMBER(ImGuiIO, ConfigFlags)
        // ImGuiConfigFlags BackendFlags;          // = 0                  // Set ImGuiBackendFlags_ enum. Set by imgui_impl_xxx files or custom back-end.
        CLASS_MEMBER(ImGuiIO, BackendFlags)
        // ImVec2        DisplaySize;              // <unset>              // Display size, in pixels. For clamping windows positions.
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiIO, DisplaySize)
        // float         DeltaTime;                // = 1.0f/60.0f         // Time elapsed since last frame, in seconds.
        CLASS_MEMBER(ImGuiIO, DeltaTime)
        // float         IniSavingRate;            // = 5.0f               // Maximum time between saving positions/sizes to .ini file, in seconds.
        CLASS_MEMBER(ImGuiIO, IniSavingRate)
        // const char*   IniFilename;              // = "imgui.ini"        // Path to .ini file. NULL to disable .ini saving.
        CLASS_MEMBER_GET_SET(ImGuiIO, IniFilename, 
            {
                return that.IniFilename == NULL ? emscripten::val::null() : emscripten::val(that.IniFilename);
            },
            {
                WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
                ctx->_ImGuiIO_IniFilename = value.as<std::string>(); that.IniFilename = value.isNull() ? NULL : ctx->_ImGuiIO_IniFilename.c_str();
            }
        )
        // const char*   LogFilename;              // = "imgui_log.txt"    // Path to .log file (default parameter to ImGui::LogToFile when no file is specified).
        CLASS_MEMBER_GET_SET(ImGuiIO, LogFilename, 
            {
                return that.LogFilename == NULL ? emscripten::val::null() : emscripten::val(that.LogFilename);
            },
            {
                WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
                ctx->_ImGuiIO_LogFilename = value.as<std::string>(); that.LogFilename = value.isNull() ? NULL : ctx->_ImGuiIO_LogFilename.c_str();
            }
        )
        // float         MouseDoubleClickTime;     // = 0.30f              // Time for a double-click, in seconds.
        CLASS_MEMBER(ImGuiIO, MouseDoubleClickTime)
        // float         MouseDoubleClickMaxDist;  // = 6.0f               // Distance threshold to stay in to validate a double-click, in pixels.
        CLASS_MEMBER(ImGuiIO, MouseDoubleClickMaxDist)
        // float         MouseDragThreshold;       // = 6.0f               // Distance threshold before considering we are dragging
        CLASS_MEMBER(ImGuiIO, MouseDragThreshold)
        // KeyMap[] removed in v1.92 - use AddKeyEvent() instead
        // float         KeyRepeatDelay;           // = 0.250f             // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
        CLASS_MEMBER(ImGuiIO, KeyRepeatDelay)
        // float         KeyRepeatRate;            // = 0.050f             // When holding a key/button, rate at which it repeats, in seconds.
        CLASS_MEMBER(ImGuiIO, KeyRepeatRate)
        // void*         UserData;                 // = NULL               // Store your own data for retrieval by callbacks.
        CLASS_MEMBER_GET_SET(ImGuiIO, UserData, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_UserData; }, 
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_UserData = value; })

        // ImFontAtlas*  Fonts;                    // <auto>               // Load and assemble one or more fonts into a single tightly packed texture. Output to Fonts array.
        CLASS_MEMBER_GET_RAW_POINTER(ImGuiIO, Fonts)
        // float         FontGlobalScale;          // = 1.0f               // Global scale all fonts
        CLASS_MEMBER(ImGuiIO, FontGlobalScale)
        // bool          FontAllowUserScaling;     // = false              // Allow user scaling text of individual window with CTRL+Wheel.
        CLASS_MEMBER(ImGuiIO, FontAllowUserScaling)
        // ImFont*       FontDefault;              // = NULL               // Font to use on NewFrame(). Use NULL to uses Fonts->Fonts[0].
        CLASS_MEMBER_GET_SET_RAW_POINTER(ImGuiIO, FontDefault)
        // ImVec2        DisplayFramebufferScale;  // = (1.0f,1.0f)        // For retina display or other situations where window coordinates are different from framebuffer coordinates. User storage only, presently not used by ImGui.
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiIO, DisplayFramebufferScale)

        // Docking options (when ImGuiConfigFlags_DockingEnable is set)
        // bool        ConfigDockingNoSplit;           // = false          // Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.
        CLASS_MEMBER(ImGuiIO, ConfigDockingNoSplit)
        // bool        ConfigDockingWithShift;         // = false          // Enable docking with holding Shift key (reduce visual noise, allows dropping in wider space)
        CLASS_MEMBER(ImGuiIO, ConfigDockingWithShift)
        // bool        ConfigDockingAlwaysTabBar;      // = false          // [BETA] [FIXME: This currently creates regression with auto-sizing and general overhead] Make every single floating window display within a docking node.
        CLASS_MEMBER(ImGuiIO, ConfigDockingAlwaysTabBar)
        // bool        ConfigDockingTransparentPayload;// = false
        CLASS_MEMBER(ImGuiIO, ConfigDockingTransparentPayload)
        // bool        ConfigDockingNoDockingOver;     // = false
        CLASS_MEMBER(ImGuiIO, ConfigDockingNoDockingOver)

        // Viewport options (when ImGuiConfigFlags_ViewportsEnable is set)
        CLASS_MEMBER(ImGuiIO, ConfigViewportsNoAutoMerge)
        CLASS_MEMBER(ImGuiIO, ConfigViewportsNoTaskBarIcon)
        CLASS_MEMBER(ImGuiIO, ConfigViewportsNoDecoration)
        CLASS_MEMBER(ImGuiIO, ConfigViewportsNoDefaultParent)

        // Keyboard/Gamepad Navigation options
        CLASS_MEMBER(ImGuiIO, ConfigNavSwapGamepadButtons)
        CLASS_MEMBER(ImGuiIO, ConfigNavMoveSetMousePos)
        CLASS_MEMBER(ImGuiIO, ConfigNavCaptureKeyboard)
        CLASS_MEMBER(ImGuiIO, ConfigNavEscapeClearFocusItem)
        CLASS_MEMBER(ImGuiIO, ConfigNavEscapeClearFocusWindow)
        CLASS_MEMBER(ImGuiIO, ConfigNavCursorVisibleAuto)
        CLASS_MEMBER(ImGuiIO, ConfigNavCursorVisibleAlways)

        // DPI/Scaling options
        CLASS_MEMBER(ImGuiIO, ConfigDpiScaleFonts)
        CLASS_MEMBER(ImGuiIO, ConfigDpiScaleViewports)

        // Miscellaneous options
        CLASS_MEMBER(ImGuiIO, MouseDrawCursor)
        CLASS_MEMBER(ImGuiIO, ConfigMacOSXBehaviors)
        CLASS_MEMBER(ImGuiIO, ConfigInputTrickleEventQueue)
        CLASS_MEMBER(ImGuiIO, ConfigInputTextCursorBlink)
        CLASS_MEMBER(ImGuiIO, ConfigInputTextEnterKeepActive)
        CLASS_MEMBER(ImGuiIO, ConfigDragClickToInputText)
        CLASS_MEMBER(ImGuiIO, ConfigWindowsResizeFromEdges)
        CLASS_MEMBER(ImGuiIO, ConfigWindowsMoveFromTitleBarOnly)
        CLASS_MEMBER(ImGuiIO, ConfigWindowsCopyContentsWithCtrlC)
        CLASS_MEMBER(ImGuiIO, ConfigScrollbarScrollByPage)
        CLASS_MEMBER(ImGuiIO, ConfigMemoryCompactTimer)

        // Debug options
        CLASS_MEMBER(ImGuiIO, ConfigErrorRecovery)
        CLASS_MEMBER(ImGuiIO, ConfigErrorRecoveryEnableAssert)
        CLASS_MEMBER(ImGuiIO, ConfigErrorRecoveryEnableDebugLog)
        CLASS_MEMBER(ImGuiIO, ConfigErrorRecoveryEnableTooltip)
        CLASS_MEMBER(ImGuiIO, ConfigDebugIsDebuggerPresent)
        CLASS_MEMBER(ImGuiIO, ConfigDebugHighlightIdConflicts)
        CLASS_MEMBER(ImGuiIO, ConfigDebugBeginReturnValueOnce)
        CLASS_MEMBER(ImGuiIO, ConfigDebugBeginReturnValueLoop)
        CLASS_MEMBER(ImGuiIO, ConfigDebugIgnoreFocusLoss)
        CLASS_MEMBER(ImGuiIO, ConfigDebugIniSettings)

        //------------------------------------------------------------------
        // Settings (User Functions)
        //------------------------------------------------------------------

        CLASS_MEMBER_GET_SET(ImGuiIO, BackendPlatformName, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendPlatformName; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendPlatformName = value; }
        )
        CLASS_MEMBER_GET_SET(ImGuiIO, BackendRendererName, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendRendererName; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendRendererName = value; }
        )
        CLASS_MEMBER_GET_SET(ImGuiIO, BackendPlatformUserData, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendPlatformUserData; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendPlatformUserData = value; }
        )
        CLASS_MEMBER_GET_SET(ImGuiIO, BackendRendererUserData, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendRendererUserData; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendRendererUserData = value; }
        )
        CLASS_MEMBER_GET_SET(ImGuiIO, BackendLanguageUserData, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendLanguageUserData; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_BackendLanguageUserData = value; }
        )

        // Optional: access OS clipboard
        // (default to use native Win32 clipboard on Windows, otherwise uses a private clipboard. Override to access OS clipboard on other architectures)
        // const char* (*GetClipboardTextFn)(void* user_data);
        CLASS_MEMBER_GET_SET(ImGuiIO, GetClipboardTextFn, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_GetClipboardTextFn; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_GetClipboardTextFn = value; }
        )
        // void        (*SetClipboardTextFn)(void* user_data, const char* text);
        CLASS_MEMBER_GET_SET(ImGuiIO, SetClipboardTextFn, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_SetClipboardTextFn; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_SetClipboardTextFn = value; }
        )
        // void*       ClipboardUserData;
        CLASS_MEMBER_GET_SET(ImGuiIO, ClipboardUserData, 
            { return WrapImGuiContext::GetCurrentContext()->_ImGuiIO_ClipboardUserData; },
            { WrapImGuiContext::GetCurrentContext()->_ImGuiIO_ClipboardUserData = value; }
        )

        // Optional: override memory allocations. MemFreeFn() may be called with a NULL pointer.
        // (default to posix malloc/free)
        // void*       (*MemAllocFn)(size_t sz);
        // void        (*MemFreeFn)(void* ptr);

        // Optional: notify OS Input Method Editor of the screen position of your cursor for text input position (e.g. when using Japanese/Chinese IME in Windows)
        // (default to use native imm32 api on Windows)
        // void        (*ImeSetInputScreenPosFn)(int x, int y);
        // void*       ImeWindowHandle;            // (Windows) Set this to your HWND to get automatic IME cursor positioning.

        //------------------------------------------------------------------
        // Input - Fill before calling NewFrame()
        //------------------------------------------------------------------

        // ImVec2      MousePos;                   // Mouse position, in pixels. Set to ImVec2(-FLT_MAX,-FLT_MAX) if mouse is unavailable (on another screen, etc.)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiIO, MousePos)
        // bool        MouseDown[5];               // Mouse buttons: left, right, middle + extras. ImGui itself mostly only uses left button (BeginPopupContext** are using right button). Others buttons allows us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
        .function("_getAt_MouseDown", FUNCTION(bool, (const ImGuiIO& that, int index), {
            return (0 <= index && index < IM_ARRAYSIZE(that.MouseDown)) ? that.MouseDown[index] : false;
        }), emscripten::allow_raw_pointers())
        .function("_setAt_MouseDown", FUNCTION(bool, (ImGuiIO& that, int index, bool value), {
            if (0 <= index && index < IM_ARRAYSIZE(that.MouseDown)) { that.MouseDown[index] = value; return true; } return false;
        }), emscripten::allow_raw_pointers())
        // float       MouseWheel;
        CLASS_MEMBER(ImGuiIO, MouseWheel)
        // float       MouseWheelH;
        CLASS_MEMBER(ImGuiIO, MouseWheelH)
        // bool        KeyCtrl;
        CLASS_MEMBER(ImGuiIO, KeyCtrl)
        // bool        KeyShift;
        CLASS_MEMBER(ImGuiIO, KeyShift)
        // bool        KeyAlt;
        CLASS_MEMBER(ImGuiIO, KeyAlt)
        // bool        KeySuper;
        CLASS_MEMBER(ImGuiIO, KeySuper)
        // ImGuiKeyChord KeyMods;
        CLASS_MEMBER(ImGuiIO, KeyMods)
        // KeysDown[] removed in v1.92 - use AddKeyEvent() instead
        // NavInputs[] removed in v1.92 - use AddKeyEvent() with ImGuiKey_GamepadXXX instead

        // Input Functions
        // IMGUI_API void AddKeyEvent(ImGuiKey key, bool down);
        .function("AddKeyEvent", FUNCTION(void, (ImGuiIO& that, int key, bool down), {
            that.AddKeyEvent((ImGuiKey)key, down);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void AddKeyAnalogEvent(ImGuiKey key, bool down, float v);
        .function("AddKeyAnalogEvent", FUNCTION(void, (ImGuiIO& that, int key, bool down, float v), {
            that.AddKeyAnalogEvent((ImGuiKey)key, down, v);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void AddMousePosEvent(float x, float y);
        CLASS_METHOD(ImGuiIO, AddMousePosEvent)
        // IMGUI_API void AddMouseButtonEvent(int button, bool down);
        CLASS_METHOD(ImGuiIO, AddMouseButtonEvent)
        // IMGUI_API void AddMouseWheelEvent(float wheel_x, float wheel_y);
        CLASS_METHOD(ImGuiIO, AddMouseWheelEvent)
        // IMGUI_API void AddMouseSourceEvent(ImGuiMouseSource source);
        .function("AddMouseSourceEvent", FUNCTION(void, (ImGuiIO& that, int source), {
            that.AddMouseSourceEvent((ImGuiMouseSource)source);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void AddFocusEvent(bool focused);
        CLASS_METHOD(ImGuiIO, AddFocusEvent)
        // IMGUI_API void AddInputCharacter(unsigned int c);
        CLASS_METHOD(ImGuiIO, AddInputCharacter)
        // IMGUI_API void AddInputCharacterUTF16(ImWchar16 c);
        .function("AddInputCharacterUTF16", FUNCTION(void, (ImGuiIO& that, int c), {
            that.AddInputCharacterUTF16((ImWchar16)c);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void AddInputCharactersUTF8(const char* str);
        .function("AddInputCharactersUTF8", FUNCTION(void, (ImGuiIO& that, std::string utf8_chars), {
            that.AddInputCharactersUTF8(utf8_chars.c_str());
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void SetKeyEventNativeData(ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index = -1);
        .function("SetKeyEventNativeData", FUNCTION(void, (ImGuiIO& that, int key, int native_keycode, int native_scancode), {
            that.SetKeyEventNativeData((ImGuiKey)key, native_keycode, native_scancode);
        }), emscripten::allow_raw_pointers())
        // IMGUI_API void SetAppAcceptingEvents(bool accepting_events);
        CLASS_METHOD(ImGuiIO, SetAppAcceptingEvents)
        // IMGUI_API void ClearEventsQueue();
        CLASS_METHOD(ImGuiIO, ClearEventsQueue)
        // IMGUI_API void ClearInputKeys();
        CLASS_METHOD(ImGuiIO, ClearInputKeys)
        // IMGUI_API void ClearInputMouse();
        CLASS_METHOD(ImGuiIO, ClearInputMouse)

        //------------------------------------------------------------------
        // Output - Retrieve after calling NewFrame()
        //------------------------------------------------------------------

        // bool        WantCaptureMouse;           // When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application. This is set by ImGui when it wants to use your mouse (e.g. unclicked mouse is hovering a window, or a widget is active). 
        CLASS_MEMBER(ImGuiIO, WantCaptureMouse)
        // bool        WantCaptureKeyboard;        // When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application. This is set by ImGui when it wants to use your keyboard inputs.
        CLASS_MEMBER(ImGuiIO, WantCaptureKeyboard)
        // bool        WantTextInput;              // Mobile/console: when io.WantTextInput is true, you may display an on-screen keyboard. This is set by ImGui when it wants textual keyboard input to happen (e.g. when a InputText widget is active).
        CLASS_MEMBER(ImGuiIO, WantTextInput)
        // bool        WantSetMousePos;            // [BETA-NAV] MousePos has been altered, back-end should reposition mouse on next frame. Set only when 'NavMovesMouse=true'.
        CLASS_MEMBER(ImGuiIO, WantSetMousePos)
        // bool        WantSaveIniSettings;        // When manual .ini load/save is active (io.IniFilename == NULL), this will be set to notify your application that you can call SaveIniSettingsToMemory() and save yourself. IMPORTANT: You need to clear io.WantSaveIniSettings yourself.
        CLASS_MEMBER(ImGuiIO, WantSaveIniSettings)
        // bool        NavActive;                  // Directional navigation is currently allowed (will handle ImGuiKey_NavXXX events) = a window is focused and it doesn't use the ImGuiWindowFlags_NoNavInputs flag.
        CLASS_MEMBER(ImGuiIO, NavActive)
        // bool        NavVisible;                 // Directional navigation is visible and allowed (will handle ImGuiKey_NavXXX events).
        CLASS_MEMBER(ImGuiIO, NavVisible)
        // float       Framerate;                  // Application framerate estimation, in frame per second. Solely for convenience. Rolling average estimation based on IO.DeltaTime over 120 frames
        CLASS_MEMBER(ImGuiIO, Framerate)
        // int         MetricsRenderVertices;      // Vertices output during last call to Render()
        CLASS_MEMBER(ImGuiIO, MetricsRenderVertices)
        // int         MetricsRenderIndices;       // Indices output during last call to Render() = number of triangles * 3
        CLASS_MEMBER(ImGuiIO, MetricsRenderIndices)
        // int         MetricsRenderWindows;       // Number of visible windows
        CLASS_MEMBER(ImGuiIO, MetricsRenderWindows)
        // int         MetricsActiveWindows;
        CLASS_MEMBER(ImGuiIO, MetricsActiveWindows)
        // MetricsActiveAllocations removed in v1.92
        // ImVec2      MouseDelta;                 // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiIO, MouseDelta)

        //------------------------------------------------------------------
        // [Internal] ImGui will maintain those fields. Forward compatibility not guaranteed!
        //------------------------------------------------------------------

        // ImVec2      MousePosPrev;               // Previous mouse position temporary storage (nb: not for public use, set to MousePos in NewFrame())
        // ImVec2      MouseClickedPos[5];         // Position at time of clicking
        .function("_getAt_MouseClickedPos", FUNCTION(emscripten::val, (const ImGuiIO* that, int index), {
            if (0 <= index && index < IM_ARRAYSIZE(that->MouseClickedPos)) {
                const auto p = &that->MouseClickedPos[index]; return emscripten::val(p);
            }
            return emscripten::val::undefined();
        }), emscripten::allow_raw_pointers())
        // float       MouseClickedTime[5];        // Time of last click (used to figure out double-click)
        // bool        MouseClicked[5];            // Mouse button went from !Down to Down
        // bool        MouseDoubleClicked[5];      // Has mouse button been double-clicked?
        // bool        MouseReleased[5];           // Mouse button went from Down to !Down
        // bool        MouseDownOwned[5];          // Track if button was clicked inside a window. We don't request mouse capture from the application if click started outside ImGui bounds.
        // float       MouseDownDuration[5];       // Duration the mouse button has been down (0.0f == just clicked)
        .function("_getAt_MouseDownDuration", FUNCTION(float, (const ImGuiIO& that, int index), {
            return (0 <= index && index < IM_ARRAYSIZE(that.MouseDownDuration)) ? that.MouseDownDuration[index] : -1.0f;
        }))
        // float       MouseDownDurationPrev[5];   // Previous time the mouse button has been down
        // ImVec2      MouseDragMaxDistanceAbs[5]; // Maximum distance, absolute, on each axis, of how much mouse has traveled from the clicking point
        // float       MouseDragMaxDistanceSqr[5]; // Squared maximum distance of how much mouse has traveled from the clicking point
        // ImU16       MouseClickedCount[5];       // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc.
        .function("_getAt_MouseClickedCount", FUNCTION(int, (const ImGuiIO& that, int index), {
            return (0 <= index && index < IM_ARRAYSIZE(that.MouseClickedCount)) ? (int)that.MouseClickedCount[index] : 0;
        }))
        // KeysDownDuration[] removed in v1.92 - use IsKeyDown()/GetKeyData() instead
        // NavInputsDownDuration[] removed in v1.92

        // IMGUI_API   ImGuiIO();
    ;
}

EMSCRIPTEN_BINDINGS(ImGuiStyle) {
    emscripten::class_<ImGuiStyle>("ImGuiStyle")
        // float       Alpha;                      // Global alpha applies to everything in Dear ImGui.
        // ImVec2      WindowPadding;              // Padding within a window.
        // float       WindowRounding;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
        // float       WindowBorderSize;           // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        // ImVec2      WindowMinSize;              // Minimum window size. This is a global setting. If you want to constraint individual windows, use SetNextWindowSizeConstraints().
        // ImVec2      WindowTitleAlign;           // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
        // ImGuiDir    WindowMenuButtonPosition;   // Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to ImGuiDir_Left.
        // float       ChildRounding;              // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
        // float       ChildBorderSize;            // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        // float       PopupRounding;              // Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
        // float       PopupBorderSize;            // Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        // ImVec2      FramePadding;               // Padding within a framed rectangle (used by most widgets).
        // float       FrameRounding;              // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
        // float       FrameBorderSize;            // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        // ImVec2      ItemSpacing;                // Horizontal and vertical spacing between widgets/lines.
        // ImVec2      ItemInnerSpacing;           // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
        // ImVec2      CellPadding;                // Padding within a table cell
        // ImVec2      TouchExtraPadding;          // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
        // float       IndentSpacing;              // Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
        // float       ColumnsMinSpacing;          // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
        // float       ScrollbarSize;              // Width of the vertical scrollbar, Height of the horizontal scrollbar.
        // float       ScrollbarRounding;          // Radius of grab corners for scrollbar.
        // float       GrabMinSize;                // Minimum width/height of a grab box for slider/scrollbar.
        // float       GrabRounding;               // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
        // float       LogSliderDeadzone;          // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
        // float       TabRounding;                // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
        // float       TabBorderSize;              // Thickness of border around tabs.
        // float       TabMinWidthForCloseButton;  // Minimum width for close button to appears on an unselected tab when hovered. Set to 0.0f to always show when hovering, set to FLT_MAX to never show close button unless selected.
        // ImGuiDir    ColorButtonPosition;        // Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
        // ImVec2      ButtonTextAlign;            // Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
        // ImVec2      SelectableTextAlign;        // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
        // ImVec2      DisplayWindowPadding;       // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
        // ImVec2      DisplaySafeAreaPadding;     // If you cannot see the edges of your screen (e.g. on a TV) increase the safe area padding. Apply to popups/tooltips as well regular windows. NB: Prefer configuring your TV sets correctly!
        // float       MouseCursorScale;           // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
        // bool        AntiAliasedLines;           // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
        // bool        AntiAliasedLinesUseTex;     // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering. Latched at the beginning of the frame (copied to ImDrawList).
        // bool        AntiAliasedFill;            // Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
        // float       CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
        // float       CircleSegmentMaxError;      // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.
        // ImVec4      Colors[ImGuiCol_COUNT];
        // Font scaling
        CLASS_MEMBER(ImGuiStyle, FontSizeBase)
        CLASS_MEMBER(ImGuiStyle, FontScaleMain)
        CLASS_MEMBER(ImGuiStyle, FontScaleDpi)

        CLASS_MEMBER(ImGuiStyle, Alpha)
        CLASS_MEMBER(ImGuiStyle, DisabledAlpha)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, WindowPadding)
        CLASS_MEMBER(ImGuiStyle, WindowRounding)
        CLASS_MEMBER(ImGuiStyle, WindowBorderSize)
        CLASS_MEMBER(ImGuiStyle, WindowBorderHoverPadding)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, WindowMinSize)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, WindowTitleAlign)
        // WindowMenuButtonPosition is ImGuiDir enum; cast to int to avoid embind UnboundTypeError
        CLASS_MEMBER_GET_SET(ImGuiStyle, WindowMenuButtonPosition,
            { return emscripten::val(static_cast<int>(that.WindowMenuButtonPosition)); },
            { that.WindowMenuButtonPosition = static_cast<ImGuiDir>(value.as<int>()); }
        )
        CLASS_MEMBER(ImGuiStyle, ChildRounding)
        CLASS_MEMBER(ImGuiStyle, ChildBorderSize)
        CLASS_MEMBER(ImGuiStyle, PopupRounding)
        CLASS_MEMBER(ImGuiStyle, PopupBorderSize)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, FramePadding)
        CLASS_MEMBER(ImGuiStyle, FrameRounding)
        CLASS_MEMBER(ImGuiStyle, FrameBorderSize)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, ItemSpacing)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, ItemInnerSpacing)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, CellPadding)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, TouchExtraPadding)
        CLASS_MEMBER(ImGuiStyle, IndentSpacing)
        CLASS_MEMBER(ImGuiStyle, ColumnsMinSpacing)
        CLASS_MEMBER(ImGuiStyle, ScrollbarSize)
        CLASS_MEMBER(ImGuiStyle, ScrollbarRounding)
        CLASS_MEMBER(ImGuiStyle, ScrollbarPadding)
        CLASS_MEMBER(ImGuiStyle, GrabMinSize)
        CLASS_MEMBER(ImGuiStyle, GrabRounding)
        CLASS_MEMBER(ImGuiStyle, LogSliderDeadzone)
        CLASS_MEMBER(ImGuiStyle, ImageRounding)
        CLASS_MEMBER(ImGuiStyle, ImageBorderSize)
        CLASS_MEMBER(ImGuiStyle, TabRounding)
        CLASS_MEMBER(ImGuiStyle, TabBorderSize)
        CLASS_MEMBER(ImGuiStyle, TabMinWidthBase)
        CLASS_MEMBER(ImGuiStyle, TabMinWidthShrink)
        CLASS_MEMBER(ImGuiStyle, TabCloseButtonMinWidthSelected)
        CLASS_MEMBER(ImGuiStyle, TabCloseButtonMinWidthUnselected)
        CLASS_MEMBER(ImGuiStyle, TabBarBorderSize)
        CLASS_MEMBER(ImGuiStyle, TabBarOverlineSize)
        CLASS_MEMBER(ImGuiStyle, TableAngledHeadersAngle)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, TableAngledHeadersTextAlign)
        CLASS_MEMBER(ImGuiStyle, TreeLinesFlags)
        CLASS_MEMBER(ImGuiStyle, TreeLinesSize)
        CLASS_MEMBER(ImGuiStyle, TreeLinesRounding)
        CLASS_MEMBER(ImGuiStyle, DragDropTargetRounding)
        CLASS_MEMBER(ImGuiStyle, DragDropTargetBorderSize)
        CLASS_MEMBER(ImGuiStyle, DragDropTargetPadding)
        CLASS_MEMBER(ImGuiStyle, ColorMarkerSize)
        // ColorButtonPosition is ImGuiDir enum; cast to int to avoid embind UnboundTypeError
        CLASS_MEMBER_GET_SET(ImGuiStyle, ColorButtonPosition,
            { return emscripten::val(static_cast<int>(that.ColorButtonPosition)); },
            { that.ColorButtonPosition = static_cast<ImGuiDir>(value.as<int>()); }
        )
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, ButtonTextAlign)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, SelectableTextAlign)
        CLASS_MEMBER(ImGuiStyle, SeparatorSize)
        CLASS_MEMBER(ImGuiStyle, SeparatorTextBorderSize)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, SeparatorTextAlign)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, SeparatorTextPadding)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, DisplayWindowPadding)
        CLASS_MEMBER_GET_RAW_REFERENCE(ImGuiStyle, DisplaySafeAreaPadding)
        CLASS_MEMBER(ImGuiStyle, DockingNodeHasCloseButton)
        CLASS_MEMBER(ImGuiStyle, DockingSeparatorSize)
        CLASS_MEMBER(ImGuiStyle, MouseCursorScale)
        CLASS_MEMBER(ImGuiStyle, AntiAliasedLines)
        CLASS_MEMBER(ImGuiStyle, AntiAliasedLinesUseTex)
        CLASS_MEMBER(ImGuiStyle, AntiAliasedFill)
        CLASS_MEMBER(ImGuiStyle, CurveTessellationTol)
        CLASS_MEMBER(ImGuiStyle, CircleTessellationMaxError)

        // Behaviors
        CLASS_MEMBER(ImGuiStyle, HoverStationaryDelay)
        CLASS_MEMBER(ImGuiStyle, HoverDelayShort)
        CLASS_MEMBER(ImGuiStyle, HoverDelayNormal)
        CLASS_MEMBER(ImGuiStyle, HoverFlagsForTooltipMouse)
        CLASS_MEMBER(ImGuiStyle, HoverFlagsForTooltipNav)
        .function("_getAt_Colors", FUNCTION(emscripten::val, (ImGuiStyle* that, ImGuiCol index), {
            if (0 <= index && index < ImGuiCol_COUNT) {
                auto p = &that->Colors[index]; return emscripten::val(p);
            }
            return emscripten::val::undefined();
        }), emscripten::allow_raw_pointers())
        .function("_setAt_Colors", FUNCTION(bool, (ImGuiStyle* that, ImGuiCol index, emscripten::val value), {
            if (0 <= index && index < ImGuiCol_COUNT) { that->Colors[index] = import_ImVec4(value); return true; } return false;    
        }), emscripten::allow_raw_pointers())

        // IMGUI_API ImGuiStyle();
        .constructor()
        // IMGUI_API void ScaleAllSizes(float scale_factor);
        CLASS_METHOD(ImGuiStyle, ScaleAllSizes)
    ;
}

namespace ImGui {

template <typename T>
bool DragScalarV(const char* label, ImGuiDataType data_type, std::vector<T>& v, float v_speed, const void* v_min = NULL, const void* v_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0) {
    return ImGui::DragScalarN(label, data_type, v.data(), v.size(), v_speed, v_min, v_max, format, flags);
}

template <typename T>
bool SliderScalarV(const char* label, ImGuiDataType data_type, std::vector<T>& v, const void* v_min, const void* v_max, const char* format = NULL, ImGuiSliderFlags flags = 0) {
    return ImGui::SliderScalarN(label, data_type, v.data(), v.size(), v_min, v_max, format, flags);
}

template <typename T>
bool InputScalarV(const char* label, ImGuiDataType data_type, std::vector<T>& v, const void* step = NULL, const void* step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0) {
    return ImGui::InputScalarN(label, data_type, v.data(), v.size(), step, step_fast, format, flags);
}

} // namespace ImGui

//-----------------------------------------------------------------------------
// [SECTION] Dear ImGui end-user API functions
// (Note that ImGui:: being a namespace, you can add extra ImGui:: functions in your own separate file. Please don't modify imgui source files!)
//-----------------------------------------------------------------------------

// namespace ImGui
// {

// } // namespace ImGui

EMSCRIPTEN_BINDINGS(ImGui) {
    emscripten::constant("IMGUI_VERSION", std::string(IMGUI_VERSION));

    emscripten::function("IMGUI_CHECKVERSION", FUNCTION(bool, (), { return IMGUI_CHECKVERSION(); }));

    emscripten::constant("ImGuiIOSize", sizeof(ImGuiIO));
    emscripten::constant("ImGuiStyleSize", sizeof(ImGuiStyle));
    emscripten::constant("ImVec2Size", sizeof(ImVec2));
    emscripten::constant("ImVec4Size", sizeof(ImVec4));

    emscripten::constant("ImDrawVertSize", sizeof(ImDrawVert));
    emscripten::constant("ImDrawIdxSize", sizeof(ImDrawIdx));
    emscripten::constant("ImDrawVertPosOffset", (int)offsetof(ImDrawVert, pos));
    emscripten::constant("ImDrawVertUVOffset", (int)offsetof(ImDrawVert, uv));
    emscripten::constant("ImDrawVertColOffset", (int)offsetof(ImDrawVert, col));

    // Context creation and access
    // Each context create its own ImFontAtlas by default. You may instance one yourself and pass it to CreateContext() to share a font atlas between imgui contexts.
    // None of those functions is reliant on the current context.
    // IMGUI_API ImGuiContext* CreateContext(ImFontAtlas* shared_font_atlas = NULL);
    // IMGUI_API void          DestroyContext(ImGuiContext* ctx = NULL);   // NULL = destroy current context
    // IMGUI_API ImGuiContext* GetCurrentContext();
    // IMGUI_API void          SetCurrentContext(ImGuiContext* ctx);
    emscripten::function("CreateContext", FUNCTION(WrapImGuiContext*, (ImFontAtlas* shared_font_atlas), { return WrapImGuiContext::CreateContext(shared_font_atlas); }), emscripten::allow_raw_pointers()); // TODO: shared font atlas
    emscripten::function("DestroyContext", FUNCTION(void, (WrapImGuiContext* wrap), { WrapImGuiContext::DestroyContext(wrap); }), emscripten::allow_raw_pointers());
    emscripten::function("GetCurrentContext", FUNCTION(WrapImGuiContext*, (), { return WrapImGuiContext::GetCurrentContext(); }), emscripten::allow_raw_pointers());
    emscripten::function("SetCurrentContext", FUNCTION(void, (WrapImGuiContext* wrap), { WrapImGuiContext::SetCurrentContext(wrap); }), emscripten::allow_raw_pointers());

    // Main
    // IMGUI_API ImGuiIO&      GetIO();                                    // access the IO structure (mouse/keyboard/gamepad inputs, time, various configuration options/flags)
    // IMGUI_API ImGuiStyle&   GetStyle();                                 // access the Style structure (colors, sizes). Always use PushStyleCol(), PushStyleVar() to modify style mid-frame!
    // IMGUI_API void          NewFrame();                                 // start a new Dear ImGui frame, you can submit any command from this point until Render()/EndFrame().
    // IMGUI_API void          EndFrame();                                 // ends the Dear ImGui frame. automatically called by Render(). If you don't need to render data (skipping rendering) you may call EndFrame() without Render()... but you'll have wasted CPU already! If you don't need to render, better to not create any windows and not call NewFrame() at all!
    // IMGUI_API void          Render();                                   // ends the Dear ImGui frame, finalize the draw data. You can then get call GetDrawData().
    // IMGUI_API ImDrawData*   GetDrawData();                              // valid after Render() and until the next call to NewFrame(). this is what you have to render.
    emscripten::function("GetIO", FUNCTION(emscripten::val, (), { ImGuiIO* p = &ImGui::GetIO(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("GetStyle", FUNCTION(emscripten::val, (), { ImGuiStyle* p = &ImGui::GetStyle(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("NewFrame", &ImGui::NewFrame);
    emscripten::function("EndFrame", &ImGui::EndFrame);
    emscripten::function("Render", &ImGui::Render);
    emscripten::function("GetDrawData", FUNCTION(emscripten::val, (), { ImDrawData* p = ImGui::GetDrawData(); return emscripten::val(p); }), emscripten::allow_raw_pointers());

    // Demo, Debug, Information
    // IMGUI_API void          ShowDemoWindow(bool* p_open = NULL);        // create Demo window. demonstrate most ImGui features. call this to learn about the library! try to make it always available in your application!
    // IMGUI_API void          ShowMetricsWindow(bool* p_open = NULL);     // create Metrics/Debugger window. display Dear ImGui internals: windows, draw commands, various internal state, etc.
    // IMGUI_API void          ShowAboutWindow(bool* p_open = NULL);       // create About window. display Dear ImGui version, credits and build/system information.
    // IMGUI_API void          ShowStyleEditor(ImGuiStyle* ref = NULL);    // add style editor block (not a window). you can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it uses the default style)
    // IMGUI_API bool          ShowStyleSelector(const char* label);       // add style selector block (not a window), essentially a combo listing the default styles.
    // IMGUI_API void          ShowFontSelector(const char* label);        // add font selector block (not a window), essentially a combo listing the loaded fonts.
    // IMGUI_API void          ShowUserGuide();                            // add basic help/info block (not a window): how to manipulate ImGui as a end-user (mouse/keyboard controls).
    // IMGUI_API const char*   GetVersion();                               // get the compiled version string e.g. "1.80 WIP" (essentially the value for IMGUI_VERSION from the compiled version of imgui.cpp)
    emscripten::function("ShowDemoWindow", FUNCTION(void, (emscripten::val p_open), { ImGui::ShowDemoWindow(access_maybe_null_value<bool>(p_open)); }));
    emscripten::function("ShowMetricsWindow", FUNCTION(void, (emscripten::val p_open), { ImGui::ShowMetricsWindow(access_maybe_null_value<bool>(p_open)); }));
    emscripten::function("ShowAboutWindow", FUNCTION(void, (emscripten::val p_open), { ImGui::ShowAboutWindow(access_maybe_null_value<bool>(p_open)); }));
    emscripten::function("ShowDebugLogWindow", FUNCTION(void, (emscripten::val p_open), { ImGui::ShowDebugLogWindow(access_maybe_null_value<bool>(p_open)); }));
    emscripten::function("ShowIDStackToolWindow", FUNCTION(void, (emscripten::val p_open), { ImGui::ShowIDStackToolWindow(access_maybe_null_value<bool>(p_open)); }));
    emscripten::function("ShowStyleEditor", FUNCTION(void, (emscripten::val ref), { ImGui::ShowStyleEditor(ref.isNull() ? NULL : ref.as<ImGuiStyle*>(emscripten::allow_raw_pointers())); }));
    emscripten::function("ShowStyleSelector", FUNCTION(bool, (std::string label), { return ImGui::ShowStyleSelector(label.c_str()); }));
    emscripten::function("ShowFontSelector", FUNCTION(void, (std::string label), { ImGui::ShowFontSelector(label.c_str()); }));
    emscripten::function("ShowUserGuide", &ImGui::ShowUserGuide);
    emscripten::function("GetVersion", FUNCTION(std::string, (), { return ImGui::GetVersion(); }));

    // Styles
    // IMGUI_API void          StyleColorsDark(ImGuiStyle* dst = NULL);    // new, recommended style (default)
    // IMGUI_API void          StyleColorsLight(ImGuiStyle* dst = NULL);   // best used with borders and a custom, thicker font
    // IMGUI_API void          StyleColorsClassic(ImGuiStyle* dst = NULL); // classic imgui style
    emscripten::function("StyleColorsDark", FUNCTION(void, (ImGuiStyle* dst), { ImGui::StyleColorsDark(dst); }), emscripten::allow_raw_pointers());
    emscripten::function("StyleColorsLight", FUNCTION(void, (ImGuiStyle* dst), { ImGui::StyleColorsLight(dst); }), emscripten::allow_raw_pointers());
    emscripten::function("StyleColorsClassic", FUNCTION(void, (ImGuiStyle* dst), { ImGui::StyleColorsClassic(dst); }), emscripten::allow_raw_pointers());

    // Windows
    // - Begin() = push window to the stack and start appending to it. End() = pop window from the stack.
    // - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
    //   which clicking will set the boolean to false when clicked.
    // - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times.
    //   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
    // - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
    //   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
    //   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
    //    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
    //    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    // - Note that the bottom of window stack always contains a window called "Debug".
    // IMGUI_API bool          Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
    // IMGUI_API void          End();
    emscripten::function("Begin", FUNCTION(bool, (std::string label, emscripten::val p_open, ImGuiWindowFlags flags), { return ImGui::Begin(label.c_str(), access_maybe_null_value<bool>(p_open), flags); }));
    emscripten::function("End", &ImGui::End);

    // Child Windows
    // - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
    // - For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed size / <0.0f: use remaining window size minus abs(size) / Each axis can use a different mode, e.g. ImVec2(0,400).
    // - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.
    //   Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
    //   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
    //    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
    //    returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    // IMGUI_API bool          BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
    // IMGUI_API bool          BeginChild(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
    // IMGUI_API void          EndChild();
    emscripten::function("BeginChild", FUNCTION(bool, (emscripten::val id, emscripten::val size, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags), {
        if (id.typeOf().strictlyEquals(emscripten::val("string"))) {
            return ImGui::BeginChild(id.as<std::string>().c_str(), import_ImVec2(size), child_flags, window_flags);
        } else {
            return ImGui::BeginChild(id.as<ImGuiID>(), import_ImVec2(size), child_flags, window_flags);
        }
    }));
    emscripten::function("EndChild", &ImGui::EndChild);

    // Windows Utilities
    // - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next window' = next window we will Begin() into.
    // IMGUI_API bool          IsWindowAppearing();
    // IMGUI_API bool          IsWindowCollapsed();
    // IMGUI_API bool          IsWindowFocused(ImGuiFocusedFlags flags=0); // is current window focused? or its root/child, depending on flags. see flags for options.
    // IMGUI_API bool          IsWindowHovered(ImGuiHoveredFlags flags=0); // is current window hovered (and typically: not blocked by a popup/modal)? see flags for options. NB: If you are trying to check whether your mouse should be dispatched to imgui or to your app, you should use the 'io.WantCaptureMouse' boolean for that! Please read the FAQ!
    // IMGUI_API ImDrawList*   GetWindowDrawList();                        // get draw list associated to the current window, to append your own drawing primitives
    // IMGUI_API ImVec2        GetWindowPos();                             // get current window position in screen space (useful if you want to do your own drawing via the DrawList API)
    // IMGUI_API ImVec2        GetWindowSize();                            // get current window size
    // IMGUI_API float         GetWindowWidth();                           // get current window width (shortcut for GetWindowSize().x)
    // IMGUI_API float         GetWindowHeight();                          // get current window height (shortcut for GetWindowSize().y)
    emscripten::function("IsWindowAppearing", &ImGui::IsWindowAppearing);
    emscripten::function("IsWindowCollapsed", &ImGui::IsWindowCollapsed);
    emscripten::function("IsWindowFocused", &ImGui::IsWindowFocused);
    emscripten::function("IsWindowHovered", &ImGui::IsWindowHovered);
    emscripten::function("GetWindowDrawList", FUNCTION(emscripten::val, (), { ImDrawList* p = ImGui::GetWindowDrawList(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("GetWindowPos", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetWindowPos(), out); }));
    emscripten::function("GetWindowSize", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetWindowSize(), out); }));
    emscripten::function("GetWindowDpiScale", &ImGui::GetWindowDpiScale);
    emscripten::function("GetWindowWidth", &ImGui::GetWindowWidth);
    emscripten::function("GetWindowHeight", &ImGui::GetWindowHeight);
    emscripten::function("GetWindowViewport", FUNCTION(emscripten::val, (), {
        ImGuiViewport* p = ImGui::GetWindowViewport();
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());

    // Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
    // IMGUI_API void          SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
    // IMGUI_API void          SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
    // IMGUI_API void          SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down. Use callback to apply non-trivial programmatic constraints.
    // IMGUI_API void          SetNextWindowContentSize(const ImVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
    // IMGUI_API void          SetNextWindowCollapsed(bool collapsed, ImGuiCond cond = 0);                 // set next window collapsed state. call before Begin()
    // IMGUI_API void          SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
    // IMGUI_API void          SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.
    // IMGUI_API void          SetWindowPos(const ImVec2& pos, ImGuiCond cond = 0);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
    // IMGUI_API void          SetWindowSize(const ImVec2& size, ImGuiCond cond = 0);                      // (not recommended) set current window size - call within Begin()/End(). set to ImVec2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
    // IMGUI_API void          SetWindowCollapsed(bool collapsed, ImGuiCond cond = 0);                     // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
    // IMGUI_API void          SetWindowFocus();                                                           // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().
    // IMGUI_API void          SetWindowFontScale(float scale);                                            // set font scale. Adjust IO.FontGlobalScale if you want to scale all windows. This is an old API! For correct scaling, prefer to reload font + rebuild ImFontAtlas + call style.ScaleAllSizes().
    // IMGUI_API void          SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond = 0);      // set named window position.
    // IMGUI_API void          SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond = 0);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
    // IMGUI_API void          SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond = 0);   // set named window collapsed state
    // IMGUI_API void          SetWindowFocus(const char* name);                                           // set named window to be focused / top-most. use NULL to remove focus.
    emscripten::function("SetNextWindowPos", FUNCTION(void, (emscripten::val pos, ImGuiCond cond, emscripten::val pivot), { ImGui::SetNextWindowPos(import_ImVec2(pos), cond, import_ImVec2(pivot)); }));
    emscripten::function("SetNextWindowSize", FUNCTION(void, (emscripten::val size, ImGuiCond cond), { ImGui::SetNextWindowSize(import_ImVec2(size), cond); }));
    emscripten::function("SetNextWindowSizeConstraints", FUNCTION(void, (emscripten::val size_min, emscripten::val size_max, emscripten::val custom_callback, emscripten::val custom_callback_data), {
        if (!custom_callback.isNull()) {
            WrapImGuiContext::GetCurrentContext()->_ImGui_SetNextWindowSizeConstraints_custom_callback = custom_callback;
            ImGui::SetNextWindowSizeConstraints(import_ImVec2(size_min), import_ImVec2(size_max), FUNCTION(void, (ImGuiSizeCallbackData* data), {
                WrapImGuiContext::GetCurrentContext()->_ImGui_SetNextWindowSizeConstraints_custom_callback(emscripten::val(data));
            }), NULL);
        } else {
            ImGui::SetNextWindowSizeConstraints(import_ImVec2(size_min), import_ImVec2(size_max));
        }
    }));
    emscripten::function("SetNextWindowContentSize", FUNCTION(void, (emscripten::val size), { ImGui::SetNextWindowContentSize(import_ImVec2(size)); }));
    emscripten::function("SetNextWindowScroll", FUNCTION(void, (emscripten::val scroll), { ImGui::SetNextWindowScroll(import_ImVec2(scroll)); }));
    emscripten::function("SetNextWindowCollapsed", &ImGui::SetNextWindowCollapsed);
    emscripten::function("SetNextWindowFocus", &ImGui::SetNextWindowFocus);
    emscripten::function("SetNextWindowBgAlpha", &ImGui::SetNextWindowBgAlpha);
    // IMGUI_API void          SetNextWindowViewport(ImGuiID viewport_id);                                 // set next window viewport
    emscripten::function("SetNextWindowViewport", &ImGui::SetNextWindowViewport);
    emscripten::function("SetWindowPos", FUNCTION(void, (emscripten::val pos, ImGuiCond cond), { ImGui::SetWindowPos(import_ImVec2(pos), cond); }));
    emscripten::function("SetWindowSize", FUNCTION(void, (emscripten::val pos, ImGuiCond cond), { ImGui::SetWindowSize(import_ImVec2(pos), cond); }));
    emscripten::function("SetWindowCollapsed", FUNCTION(void, (bool collapsed, ImGuiCond cond), { ImGui::SetWindowCollapsed(collapsed, cond); }));
    emscripten::function("SetWindowFocus", FUNCTION(void, (), { ImGui::SetWindowFocus(); }));
    emscripten::function("SetWindowFontScale", &ImGui::SetWindowFontScale);
    emscripten::function("SetWindowNamePos", FUNCTION(void, (std::string name, emscripten::val pos, ImGuiCond cond), { ImGui::SetWindowPos(name.c_str(), import_ImVec2(pos), cond); }));
    emscripten::function("SetWindowNameSize", FUNCTION(void, (std::string name, emscripten::val size, ImGuiCond cond), { ImGui::SetWindowSize(name.c_str(), import_ImVec2(size), cond); }));
    emscripten::function("SetWindowNameCollapsed", FUNCTION(void, (std::string name, bool collapsed, ImGuiCond cond), { ImGui::SetWindowCollapsed(name.c_str(), collapsed, cond); }));
    emscripten::function("SetWindowNameFocus", FUNCTION(void, (std::string name), { ImGui::SetWindowFocus(name.c_str()); }));

    // Content region
    // - Retrieve available space from a given point. GetContentRegionAvail() is frequently useful.
    // - Those functions are bound to be redesigned (they are confusing, incomplete and the Min/Max return values are in local window coordinates which increases confusion)
    // IMGUI_API ImVec2        GetContentRegionAvail();                                        // == GetContentRegionMax() - GetCursorPos()
    // IMGUI_API ImVec2        GetContentRegionMax();                                          // current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates
    // IMGUI_API ImVec2        GetWindowContentRegionMin();                                    // content boundaries min (roughly (0,0)-Scroll), in window coordinates
    // IMGUI_API ImVec2        GetWindowContentRegionMax();                                    // content boundaries max (roughly (0,0)+Size-Scroll) where Size can be override with SetNextWindowContentSize(), in window coordinates
    // IMGUI_API float         GetWindowContentRegionWidth();                                  //
    emscripten::function("GetContentRegionAvail", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetContentRegionAvail(), out); }));
    emscripten::function("GetContentRegionMax", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetContentRegionMax(), out); }));
    emscripten::function("GetWindowContentRegionMin", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetWindowContentRegionMin(), out); }));
    emscripten::function("GetWindowContentRegionMax", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetWindowContentRegionMax(), out); }));

    // Windows Scrolling
    // IMGUI_API float         GetScrollX();                                                   // get scrolling amount [0 .. GetScrollMaxX()]
    // IMGUI_API float         GetScrollY();                                                   // get scrolling amount [0 .. GetScrollMaxY()]
    // IMGUI_API void          SetScrollX(float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
    // IMGUI_API void          SetScrollY(float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
    // IMGUI_API float         GetScrollMaxX();                                                // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
    // IMGUI_API float         GetScrollMaxY();                                                // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
    // IMGUI_API void          SetScrollHereX(float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    // IMGUI_API void          SetScrollHereY(float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    // IMGUI_API void          SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
    // IMGUI_API void          SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
    emscripten::function("GetScrollX", &ImGui::GetScrollX);
    emscripten::function("GetScrollY", &ImGui::GetScrollY);
    emscripten::function("SetScrollX", &ImGui::SetScrollX);
    emscripten::function("SetScrollY", &ImGui::SetScrollY);
    emscripten::function("GetScrollMaxX", &ImGui::GetScrollMaxX);
    emscripten::function("GetScrollMaxY", &ImGui::GetScrollMaxY);
    emscripten::function("SetScrollHereX", &ImGui::SetScrollHereX);
    emscripten::function("SetScrollHereY", &ImGui::SetScrollHereY);
    emscripten::function("SetScrollFromPosX", &ImGui::SetScrollFromPosX);
    emscripten::function("SetScrollFromPosY", &ImGui::SetScrollFromPosY);

    // Parameters stacks (shared)
    // IMGUI_API void          PushFont(ImFont* font);                                         // use NULL as a shortcut to push default font
    // IMGUI_API void          PopFont();
    // IMGUI_API void          PushStyleColor(ImGuiCol idx, ImU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
    // IMGUI_API void          PushStyleColor(ImGuiCol idx, const ImVec4& col);
    // IMGUI_API void          PopStyleColor(int count = 1);
    // IMGUI_API void          PushStyleVar(ImGuiStyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame().
    // IMGUI_API void          PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);             // modify a style ImVec2 variable. always use this if you modify the style after NewFrame().
    // IMGUI_API void          PopStyleVar(int count = 1);
    // IMGUI_API void          PushAllowKeyboardFocus(bool allow_keyboard_focus);              // allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets
    // IMGUI_API void          PopAllowKeyboardFocus();
    // IMGUI_API void          PushButtonRepeat(bool repeat);                                  // in 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.
    // IMGUI_API void          PopButtonRepeat();
    emscripten::function("PushFont", FUNCTION(void, (emscripten::val font, float font_size_base_unscaled), {
        ImFont* _font = font.isNull() ? NULL : font.as<ImFont*>(emscripten::allow_raw_pointers());
        ImGui::PushFont(_font, font_size_base_unscaled);
    }));
    emscripten::function("PopFont", &ImGui::PopFont);
    emscripten::function("PushStyleColor", FUNCTION(void, (ImGuiCol idx, emscripten::val col), {
        if (col.typeOf().strictlyEquals(emscripten::val("number"))) {
            ImGui::PushStyleColor(idx, col.as<ImU32>());
        } else {
            ImGui::PushStyleColor(idx, import_ImVec4(col));
        }
    }));
    emscripten::function("PopStyleColor", &ImGui::PopStyleColor);
    emscripten::function("PushStyleVar", FUNCTION(void, (ImGuiStyleVar idx, emscripten::val var), {
        if (var.typeOf().strictlyEquals(emscripten::val("number"))) {
            ImGui::PushStyleVar(idx, import_value<float>(var));
        } else {
            ImGui::PushStyleVar(idx, import_ImVec2(var));
        }
    }));
    emscripten::function("PopStyleVar", &ImGui::PopStyleVar);
    emscripten::function("PushStyleVarX", FUNCTION(void, (ImGuiStyleVar idx, float val_x), { ImGui::PushStyleVarX(idx, val_x); }));
    emscripten::function("PushStyleVarY", FUNCTION(void, (ImGuiStyleVar idx, float val_y), { ImGui::PushStyleVarY(idx, val_y); }));
    // PushAllowKeyboardFocus/PopAllowKeyboardFocus removed in v1.92 - use PushItemFlag/PopItemFlag instead
    emscripten::function("PushAllowKeyboardFocus", FUNCTION(void, (bool allow_keyboard_focus), {
        if (!allow_keyboard_focus) ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, true);
        else ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, false);
    }));
    emscripten::function("PopAllowKeyboardFocus", FUNCTION(void, (), { ImGui::PopItemFlag(); }));
    emscripten::function("PushButtonRepeat", &ImGui::PushButtonRepeat);
    emscripten::function("PopButtonRepeat", &ImGui::PopButtonRepeat);

    // Parameters stacks (current window)
    // IMGUI_API void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side). 0.0f = default to ~2/3 of windows width,
    // IMGUI_API void          PopItemWidth();
    // IMGUI_API void          SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
    // IMGUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
    // IMGUI_API void          PushTextWrapPos(float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
    // IMGUI_API void          PopTextWrapPos();
    emscripten::function("PushItemWidth", &ImGui::PushItemWidth);
    emscripten::function("PopItemWidth", &ImGui::PopItemWidth);
    emscripten::function("SetNextItemWidth", &ImGui::SetNextItemWidth);
    emscripten::function("CalcItemWidth", &ImGui::CalcItemWidth);
    emscripten::function("PushTextWrapPos", &ImGui::PushTextWrapPos);
    emscripten::function("PopTextWrapPos", &ImGui::PopTextWrapPos);
    // PushItemFlag / PopItemFlag
    emscripten::function("PushItemFlag", FUNCTION(void, (int flag, bool enabled), { ImGui::PushItemFlag((ImGuiItemFlags)flag, enabled); }));
    emscripten::function("PopItemFlag", &ImGui::PopItemFlag);

    // Style read access
    // IMGUI_API ImFont*       GetFont();                                                      // get current font
    // IMGUI_API float         GetFontSize();                                                  // get current font size (= height in pixels) of current font with current scale applied
    // IMGUI_API ImVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API
    // IMGUI_API ImU32         GetColorU32(ImGuiCol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for ImDrawList
    // IMGUI_API ImU32         GetColorU32(const ImVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
    // IMGUI_API ImU32         GetColorU32(ImU32 col);                                         // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
    // IMGUI_API const ImVec4& GetStyleColorVec4(ImGuiCol idx);                                // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.
    emscripten::function("GetFont", FUNCTION(emscripten::val, (), { ImFont* p = ImGui::GetFont(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("GetFontSize", &ImGui::GetFontSize);
    emscripten::function("GetFontBaked", FUNCTION(emscripten::val, (), {
        ImFontBaked* p = ImGui::GetFontBaked();
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    emscripten::function("GetFontTexUvWhitePixel", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetFontTexUvWhitePixel(), out); }));
    emscripten::function("GetColorU32_A", FUNCTION(ImU32, (ImGuiCol idx, emscripten::val alpha_mul), { return ImGui::GetColorU32(idx, import_value<float>(alpha_mul)); }));
    emscripten::function("GetColorU32_B", FUNCTION(ImU32, (emscripten::val col), { return ImGui::GetColorU32(import_ImVec4(col)); }));
    emscripten::function("GetColorU32_C", FUNCTION(ImU32, (ImU32 col), { return ImGui::GetColorU32(col); }));
    emscripten::function("GetStyleColorVec4", FUNCTION(emscripten::val, (ImGuiCol idx), { const ImVec4* p = &ImGui::GetStyleColorVec4(idx); return emscripten::val(p); }));

    // Cursor / Layout
    // - By "cursor" we mean the current output position.
    // - The typical widget behavior is to output themselves at the current cursor position, then move the cursor one line down.
    // - You can call SameLine() between widgets to undo the last carriage return and output at the right of the preceding widget.
    // - Attention! We currently have inconsistencies between window-local and absolute positions we will aim to fix with future API:
    //    Window-local coordinates:   SameLine(), GetCursorPos(), SetCursorPos(), GetCursorStartPos(), GetContentRegionMax(), GetWindowContentRegion*(), PushTextWrapPos()
    //    Absolute coordinate:        GetCursorScreenPos(), SetCursorScreenPos(), all ImDrawList:: functions.
    // IMGUI_API void          Separator();                                                    // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
    // IMGUI_API void          SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
    // IMGUI_API void          NewLine();                                                      // undo a SameLine() or force a new line when in an horizontal-layout context.
    // IMGUI_API void          Spacing();                                                      // add vertical spacing.
    // IMGUI_API void          Dummy(const ImVec2& size);                                      // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
    // IMGUI_API void          Indent(float indent_w = 0.0f);                                  // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
    // IMGUI_API void          Unindent(float indent_w = 0.0f);                                // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
    // IMGUI_API void          BeginGroup();                                                   // lock horizontal starting position
    // IMGUI_API void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
    // IMGUI_API ImVec2        GetCursorPos();                                                 // cursor position in window coordinates (relative to window position)
    // IMGUI_API float         GetCursorPosX();                                                //   (some functions are using window-relative coordinates, such as: GetCursorPos, GetCursorStartPos, GetContentRegionMax, GetWindowContentRegion* etc.
    // IMGUI_API float         GetCursorPosY();                                                //    other functions such as GetCursorScreenPos or everything in ImDrawList::
    // IMGUI_API void          SetCursorPos(const ImVec2& local_pos);                          //    are using the main, absolute coordinate system.
    // IMGUI_API void          SetCursorPosX(float local_x);                                   //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
    // IMGUI_API void          SetCursorPosY(float local_y);                                   //
    // IMGUI_API ImVec2        GetCursorStartPos();                                            // initial cursor position in window coordinates
    // IMGUI_API ImVec2        GetCursorScreenPos();                                           // cursor position in absolute screen coordinates [0..io.DisplaySize] (useful to work with ImDrawList API)
    // IMGUI_API void          SetCursorScreenPos(const ImVec2& pos);                          // cursor position in absolute screen coordinates [0..io.DisplaySize]
    // IMGUI_API void          AlignTextToFramePadding();                                      // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
    // IMGUI_API float         GetTextLineHeight();                                            // ~ FontSize
    // IMGUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    // IMGUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    // IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)
    emscripten::function("Separator", &ImGui::Separator);
    emscripten::function("SeparatorText", FUNCTION(void, (std::string label), { ImGui::SeparatorText(label.c_str()); }));
    emscripten::function("SameLine", &ImGui::SameLine);
    emscripten::function("NewLine", &ImGui::NewLine);
    emscripten::function("Spacing", &ImGui::Spacing);
    emscripten::function("Dummy", FUNCTION(void, (emscripten::val size), { ImGui::Dummy(import_ImVec2(size)); }));
    emscripten::function("Indent", &ImGui::Indent);
    emscripten::function("Unindent", &ImGui::Unindent);
    emscripten::function("BeginGroup", &ImGui::BeginGroup);
    emscripten::function("EndGroup", &ImGui::EndGroup);
    emscripten::function("GetCursorPos", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetCursorPos(), out); }));
    emscripten::function("GetCursorPosX", &ImGui::GetCursorPosX);
    emscripten::function("GetCursorPosY", &ImGui::GetCursorPosY);
    emscripten::function("SetCursorPos", FUNCTION(void, (emscripten::val local_pos), { ImGui::SetCursorPos(import_ImVec2(local_pos)); }));
    emscripten::function("SetCursorPosX", &ImGui::SetCursorPosX);
    emscripten::function("SetCursorPosY", &ImGui::SetCursorPosY);
    emscripten::function("GetCursorStartPos", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetCursorStartPos(), out); }));
    emscripten::function("GetCursorScreenPos", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetCursorScreenPos(), out); }));
    emscripten::function("SetCursorScreenPos", FUNCTION(void, (emscripten::val pos), { ImGui::SetCursorScreenPos(import_ImVec2(pos)); }));
    emscripten::function("AlignTextToFramePadding", &ImGui::AlignTextToFramePadding);
    emscripten::function("GetTextLineHeight", &ImGui::GetTextLineHeight);
    emscripten::function("GetTextLineHeightWithSpacing", &ImGui::GetTextLineHeightWithSpacing);
    emscripten::function("GetFrameHeight", &ImGui::GetFrameHeight);
    emscripten::function("GetFrameHeightWithSpacing", &ImGui::GetFrameHeightWithSpacing);

    // ID stack/scopes
    // - Read the FAQ for more details about how ID are handled in dear imgui. If you are creating widgets in a loop you most
    //   likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
    // - The resulting ID are hashes of the entire stack.
    // - You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.
    // - In this header file we use the "label"/"name" terminology to denote a string that will be displayed and used as an ID,
    //   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
    // IMGUI_API void          PushID(const char* str_id);                                     // push string into the ID stack (will hash string).
    // IMGUI_API void          PushID(const char* str_id_begin, const char* str_id_end);       // push string into the ID stack (will hash string).
    // IMGUI_API void          PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
    // IMGUI_API void          PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
    // IMGUI_API void          PopID();                                                        // pop from the ID stack.
    // IMGUI_API ImGuiID       GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into ImGuiStorage yourself
    // IMGUI_API ImGuiID       GetID(const char* str_id_begin, const char* str_id_end);
    // IMGUI_API ImGuiID       GetID(const void* ptr_id);
    emscripten::function("PushID", FUNCTION(void, (emscripten::val id), {
        if (id.typeOf().strictlyEquals(emscripten::val("number"))) {
            return ImGui::PushID(id.as<int>());
        } else {
            return ImGui::PushID(id.as<std::string>().c_str());
        }
    }));
    emscripten::function("PopID", &ImGui::PopID);
    emscripten::function("GetID", FUNCTION(ImGuiID, (emscripten::val id), {
        if (id.typeOf().strictlyEquals(emscripten::val("number"))) {
            return ImGui::GetID((const void*) id.as<int>());
        } else {
            return ImGui::GetID(id.as<std::string>().c_str());
        }
    }));

    // Widgets: Text
    // IMGUI_API void          TextUnformatted(const char* text, const char* text_end = NULL); // raw text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't require null terminated string if 'text_end' is specified, B) it's faster, no memory copy is done, no buffer size limits, recommended for long chunks of text.
    // IMGUI_API void          Text(const char* fmt, ...)                                      IM_FMTARGS(1); // formatted text
    // IMGUI_API void          TextV(const char* fmt, va_list args)                            IM_FMTLIST(1);
    // IMGUI_API void          TextColored(const ImVec4& col, const char* fmt, ...)            IM_FMTARGS(2); // shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
    // IMGUI_API void          TextColoredV(const ImVec4& col, const char* fmt, va_list args)  IM_FMTLIST(2);
    // IMGUI_API void          TextDisabled(const char* fmt, ...)                              IM_FMTARGS(1); // shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
    // IMGUI_API void          TextDisabledV(const char* fmt, va_list args)                    IM_FMTLIST(1);
    // IMGUI_API void          TextWrapped(const char* fmt, ...)                               IM_FMTARGS(1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
    // IMGUI_API void          TextWrappedV(const char* fmt, va_list args)                     IM_FMTLIST(1);
    // IMGUI_API void          LabelText(const char* label, const char* fmt, ...)              IM_FMTARGS(2); // display text+label aligned the same way as value+label widgets
    // IMGUI_API void          LabelTextV(const char* label, const char* fmt, va_list args)    IM_FMTLIST(2);
    // IMGUI_API void          BulletText(const char* fmt, ...)                                IM_FMTARGS(1); // shortcut for Bullet()+Text()
    // IMGUI_API void          BulletTextV(const char* fmt, va_list args)                      IM_FMTLIST(1);
    emscripten::function("TextUnformatted", FUNCTION(void, (std::string text), { ImGui::TextUnformatted(text.c_str(), NULL); }));
    emscripten::function("Text", FUNCTION(void, (std::string fmt), { ImGui::Text("%s", fmt.c_str()); }));
    emscripten::function("TextColored", FUNCTION(void, (emscripten::val col, std::string fmt), { ImGui::TextColored(import_ImVec4(col), "%s", fmt.c_str()); }));
    emscripten::function("TextDisabled", FUNCTION(void, (std::string fmt), { ImGui::TextDisabled("%s", fmt.c_str()); }));
    emscripten::function("TextWrapped", FUNCTION(void, (std::string fmt), { ImGui::TextWrapped("%s", fmt.c_str()); }));
    emscripten::function("LabelText", FUNCTION(void, (std::string label, std::string fmt), { ImGui::LabelText(label.c_str(), "%s", fmt.c_str()); }));
    emscripten::function("BulletText", FUNCTION(void, (std::string fmt), { ImGui::BulletText("%s", fmt.c_str()); }));
    // TextLink / TextLinkOpenURL (hyperlink text)
    emscripten::function("TextLink", FUNCTION(bool, (std::string label), { return ImGui::TextLink(label.c_str()); }));
    emscripten::function("TextLinkOpenURL", FUNCTION(void, (std::string label, std::string url), { ImGui::TextLinkOpenURL(label.c_str(), url.empty() ? NULL : url.c_str()); }));

    // Widgets: Main
    // - Most widgets return true when the value has been changed or when pressed/selected
    // - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.
    // IMGUI_API bool          Button(const char* label, const ImVec2& size = ImVec2(0, 0));   // button
    // IMGUI_API bool          SmallButton(const char* label);                                 // button with FramePadding=(0,0) to easily embed within text
    // IMGUI_API bool          InvisibleButton(const char* str_id, const ImVec2& size, ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
    // IMGUI_API bool          ArrowButton(const char* str_id, ImGuiDir dir);                  // square button with an arrow shape
    // IMGUI_API void          Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
    // IMGUI_API bool          ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
    // IMGUI_API bool          Checkbox(const char* label, bool* v);
    // IMGUI_API bool          CheckboxFlags(const char* label, int* flags, int flags_value);
    // IMGUI_API bool          CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value);
    // IMGUI_API bool          RadioButton(const char* label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
    // IMGUI_API bool          RadioButton(const char* label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
    // IMGUI_API void          ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = NULL);
    // IMGUI_API void          Bullet();                                                       // draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses
    emscripten::function("Button", FUNCTION(bool, (std::string label, emscripten::val size), { return ImGui::Button(label.c_str(), import_ImVec2(size)); }));
    emscripten::function("SmallButton", FUNCTION(bool, (std::string label), { return ImGui::SmallButton(label.c_str()); }));
    emscripten::function("InvisibleButton", FUNCTION(bool, (std::string str_id, emscripten::val size, ImGuiButtonFlags flags), { return ImGui::InvisibleButton(str_id.c_str(), import_ImVec2(size), flags); }));
    emscripten::function("ArrowButton", FUNCTION(bool, (std::string label, int dir), { return ImGui::ArrowButton(label.c_str(), (ImGuiDir)dir); }));
    emscripten::function("Image", FUNCTION(void, (emscripten::val user_texture_id, emscripten::val size, emscripten::val uv0, emscripten::val uv1), {
        ImGui::Image(ImTextureRef((ImTextureID)(intptr_t) user_texture_id.as<int>()), import_ImVec2(size), import_ImVec2(uv0), import_ImVec2(uv1));
    }));
    emscripten::function("ImageWithBg", FUNCTION(void, (emscripten::val user_texture_id, emscripten::val size, emscripten::val uv0, emscripten::val uv1, emscripten::val bg_col, emscripten::val tint_col), {
        ImGui::ImageWithBg(ImTextureRef((ImTextureID)(intptr_t) user_texture_id.as<int>()), import_ImVec2(size), import_ImVec2(uv0), import_ImVec2(uv1), import_ImVec4(bg_col), import_ImVec4(tint_col));
    }));
    emscripten::function("ImageButton", FUNCTION(bool, (std::string str_id, emscripten::val user_texture_id, emscripten::val size, emscripten::val uv0, emscripten::val uv1, emscripten::val bg_col, emscripten::val tint_col), {
        return ImGui::ImageButton(str_id.c_str(), ImTextureRef((ImTextureID)(intptr_t) user_texture_id.as<int>()), import_ImVec2(size), import_ImVec2(uv0), import_ImVec2(uv1), import_ImVec4(bg_col), import_ImVec4(tint_col));
    }));
    emscripten::function("Checkbox", FUNCTION(bool, (std::string label, emscripten::val v), { return ImGui::Checkbox(label.c_str(), access_value<bool>(v)); }));
    emscripten::function("CheckboxFlags", FUNCTION(bool, (std::string label, emscripten::val flags, unsigned int flags_value), {
        return ImGui::CheckboxFlags(label.c_str(), access_value<unsigned int>(flags), flags_value);
    }));
    emscripten::function("RadioButton_A", FUNCTION(bool, (std::string label, bool active), { return ImGui::RadioButton(label.c_str(), active); }));
    emscripten::function("RadioButton_B", FUNCTION(bool, (std::string label, emscripten::val v, int v_button), { return ImGui::RadioButton(label.c_str(), access_value<int>(v), v_button); }));
    emscripten::function("ProgressBar", FUNCTION(void, (float fraction, emscripten::val size_arg, emscripten::val overlay), { ImGui::ProgressBar(fraction, import_ImVec2(size_arg), import_maybe_null_string(overlay)); }));
    emscripten::function("Bullet", &ImGui::Bullet);

    // Widgets: Combo Box
    // - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.
    // - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose.
    // IMGUI_API bool          BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);
    // IMGUI_API void          EndCombo(); // only call EndCombo() if BeginCombo() returns true!
    // IMGUI_API bool          Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
    // IMGUI_API bool          Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
    // IMGUI_API bool          Combo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);
    emscripten::function("BeginCombo", FUNCTION(bool, (std::string label, emscripten::val preview_value, ImGuiComboFlags flags), { return ImGui::BeginCombo(label.c_str(), import_maybe_null_string(preview_value), flags); }));
    emscripten::function("EndCombo", &ImGui::EndCombo);
    emscripten::function("Combo", FUNCTION(bool, (std::string label, emscripten::val current_item, emscripten::val items_getter, emscripten::val data, int items_count, int popup_max_height_in_items), {
        WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
        ctx->_ImGui_Combo_items_getter = items_getter;
        ctx->_ImGui_Combo_data = data;
        ctx->_ImGui_Combo_items_count = items_count;
        return ImGui::Combo(label.c_str(), access_value<int>(current_item), FUNCTION(const char*, (void* user_data, int idx), {
            WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
            if (0 <= idx && idx < ctx->_ImGui_Combo_items_count) {
                emscripten::val ret = ctx->_ImGui_Combo_items_getter(ctx->_ImGui_Combo_data, emscripten::val(idx));
                ctx->_ImGui_Combo_text = ret.as<std::string>();
                return ctx->_ImGui_Combo_text.c_str();
            } else {
                return (const char*)NULL;
            }
        }), NULL, items_count, popup_max_height_in_items);
    }));

    // Widgets: Drag Sliders
    // - CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped and can go off-bounds.
    // - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every functions, note that a 'float v[X]' function argument is the same as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
    // - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits.
    // - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
    // - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
    // - Legacy: Pre-1.78 there are DragXXX() function signatures that takes a final `float power=1.0f' argument instead of the `ImGuiSliderFlags flags=0' argument.
    //   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
    // IMGUI_API bool          DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // If v_min >= v_max we have no bound
    // IMGUI_API bool          DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);  // If v_min >= v_max we have no bound
    // IMGUI_API bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);
    emscripten::function("DragFloat", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragFloat(label.c_str(), access_value<float>(v), import_value<float>(v_speed), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragFloat2", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragFloat2(label.c_str(), access_value<float, 2>(v), import_value<float>(v_speed), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragFloat3", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragFloat3(label.c_str(), access_value<float, 3>(v), import_value<float>(v_speed), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragFloat4", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragFloat4(label.c_str(), access_value<float, 4>(v), import_value<float>(v_speed), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragFloatRange2", FUNCTION(bool, (std::string label, emscripten::val v_current_min, emscripten::val v_current_max, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, emscripten::val format_max, ImGuiSliderFlags flags), {
        return ImGui::DragFloatRange2(label.c_str(), access_value<float>(v_current_min), access_value<float>(v_current_max), import_value<float>(v_speed), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), import_maybe_null_string(format_max), flags);
    }));
    emscripten::function("DragInt", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragInt(label.c_str(), access_value<int>(v), import_value<float>(v_speed), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragInt2", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragInt2(label.c_str(), access_value<int, 2>(v), import_value<float>(v_speed), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragInt3", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragInt3(label.c_str(), access_value<int, 3>(v), import_value<float>(v_speed), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragInt4", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_speed, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::DragInt4(label.c_str(), access_value<int, 4>(v), import_value<float>(v_speed), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("DragIntRange2", FUNCTION(bool, (std::string label, emscripten::val v_current_min, emscripten::val v_current_max, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, emscripten::val format_max, ImGuiSliderFlags flags), {
        return ImGui::DragIntRange2(label.c_str(), access_value<int>(v_current_min), access_value<int>(v_current_max), import_value<float>(v_speed), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), import_maybe_null_string(format_max), flags);
    }));
    emscripten::function("DragScalar", FUNCTION(bool, (std::string label, ImGuiDataType data_type, emscripten::val v, emscripten::val v_speed, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        switch (data_type) {
            case ImGuiDataType_S8:
                return ImGui::DragScalarV<ImS8>(label.c_str(), data_type, access_typed_array<ImS8>(v), import_value<float>(v_speed), import_maybe_null_value<ImS8>(v_min), import_maybe_null_value<ImS8>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U8:
                return ImGui::DragScalarV<ImU8>(label.c_str(), data_type, access_typed_array<ImU8>(v), import_value<float>(v_speed), import_maybe_null_value<ImU8>(v_min), import_maybe_null_value<ImU8>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_S16:
                return ImGui::DragScalarV<ImS16>(label.c_str(), data_type, access_typed_array<ImS16>(v), import_value<float>(v_speed), import_maybe_null_value<ImS16>(v_min), import_maybe_null_value<ImS16>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U16:
                return ImGui::DragScalarV<ImU16>(label.c_str(), data_type, access_typed_array<ImU16>(v), import_value<float>(v_speed), import_maybe_null_value<ImU16>(v_min), import_maybe_null_value<ImU16>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_S32:
                return ImGui::DragScalarV<ImS32>(label.c_str(), data_type, access_typed_array<ImS32>(v), import_value<float>(v_speed), import_maybe_null_value<ImS32>(v_min), import_maybe_null_value<ImS32>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U32:
                return ImGui::DragScalarV<ImU32>(label.c_str(), data_type, access_typed_array<ImU32>(v), import_value<float>(v_speed), import_maybe_null_value<ImU32>(v_min), import_maybe_null_value<ImU32>(v_max), import_maybe_null_string(format), flags);
            // case ImGuiDataType_S64:
            //     return ImGui::DragScalarV<ImS64>(label.c_str(), data_type, access_typed_array<ImS64>(v), import_value<float>(v_speed), import_maybe_null_value<ImS64>(v_min), import_maybe_null_value<ImS64>(v_max), import_maybe_null_string(format), flags);
            // case ImGuiDataType_U64:
            //     return ImGui::DragScalarV<ImU64>(label.c_str(), data_type, access_typed_array<ImU64>(v), import_value<float>(v_speed), import_maybe_null_value<ImU64>(v_min), import_maybe_null_value<ImU64>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_Float:
                return ImGui::DragScalarV<float>(label.c_str(), data_type, access_typed_array<float>(v), import_value<float>(v_speed), import_maybe_null_value<float>(v_min), import_maybe_null_value<float>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_Double:
                return ImGui::DragScalarV<double>(label.c_str(), data_type, access_typed_array<double>(v), import_value<float>(v_speed), import_maybe_null_value<double>(v_min), import_maybe_null_value<double>(v_max), import_maybe_null_string(format), flags);
        }
        return false;
    }));

    // Widgets: Regular Sliders
    // - CTRL+Click on any slider to turn them into an input box. Manually input values aren't clamped and can go off-bounds.
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Legacy: Pre-1.78 there are SliderXXX() function signatures that takes a final `float power=1.0f' argument instead of the `ImGuiSliderFlags flags=0' argument.
    //   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
    // IMGUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
    // IMGUI_API bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          SliderScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          VSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    emscripten::function("SliderFloat", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderFloat(label.c_str(), access_value<float>(v), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderFloat2", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderFloat2(label.c_str(), access_value<float, 2>(v), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderFloat3", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderFloat3(label.c_str(), access_value<float, 3>(v), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderFloat4", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderFloat4(label.c_str(), access_value<float, 4>(v), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderAngle", FUNCTION(bool, (std::string label, emscripten::val v_rad, emscripten::val v_degrees_min, emscripten::val v_degrees_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderAngle(label.c_str(), access_value<float>(v_rad), import_value<float>(v_degrees_min), import_value<float>(v_degrees_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderInt", FUNCTION(bool, (std::string label, emscripten::val v, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderInt(label.c_str(), access_value<int>(v), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderInt2", FUNCTION(bool, (std::string label, emscripten::val v, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderInt2(label.c_str(), access_value<int, 2>(v), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderInt3", FUNCTION(bool, (std::string label, emscripten::val v, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderInt3(label.c_str(), access_value<int, 3>(v), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderInt4", FUNCTION(bool, (std::string label, emscripten::val v, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::SliderInt4(label.c_str(), access_value<int, 4>(v), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("SliderScalar", FUNCTION(bool, (std::string label, ImGuiDataType data_type, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        switch (data_type) {
            case ImGuiDataType_S8:
                return ImGui::SliderScalarV<ImS8>(label.c_str(), data_type, access_typed_array<ImS8>(v), import_maybe_null_value<ImS8>(v_min), import_maybe_null_value<ImS8>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U8:
                return ImGui::SliderScalarV<ImU8>(label.c_str(), data_type, access_typed_array<ImU8>(v), import_maybe_null_value<ImU8>(v_min), import_maybe_null_value<ImU8>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_S16:
                return ImGui::SliderScalarV<ImS16>(label.c_str(), data_type, access_typed_array<ImS16>(v), import_maybe_null_value<ImS16>(v_min), import_maybe_null_value<ImS16>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U16:
                return ImGui::SliderScalarV<ImU16>(label.c_str(), data_type, access_typed_array<ImU16>(v), import_maybe_null_value<ImU16>(v_min), import_maybe_null_value<ImU16>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_S32:
                return ImGui::SliderScalarV<ImS32>(label.c_str(), data_type, access_typed_array<ImS32>(v), import_maybe_null_value<ImS32>(v_min), import_maybe_null_value<ImS32>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U32:
                return ImGui::SliderScalarV<ImU32>(label.c_str(), data_type, access_typed_array<ImU32>(v), import_maybe_null_value<ImU32>(v_min), import_maybe_null_value<ImU32>(v_max), import_maybe_null_string(format), flags);
            // case ImGuiDataType_S64:
            //     return ImGui::SliderScalarV<ImS64>(label.c_str(), data_type, access_typed_array<ImS64>(v), import_maybe_null_value<ImS64>(v_min), import_maybe_null_value<ImS64>(v_max), import_maybe_null_string(format), flags);
            // case ImGuiDataType_U64:
            //     return ImGui::SliderScalarV<ImU64>(label.c_str(), data_type, access_typed_array<ImU64>(v), import_maybe_null_value<ImU64>(v_min), import_maybe_null_value<ImU64>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_Float:
                return ImGui::SliderScalarV<float>(label.c_str(), data_type, access_typed_array<float>(v), import_maybe_null_value<float>(v_min), import_maybe_null_value<float>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_Double:
                return ImGui::SliderScalarV<double>(label.c_str(), data_type, access_typed_array<double>(v), import_maybe_null_value<double>(v_min), import_maybe_null_value<double>(v_max), import_maybe_null_string(format), flags);
        }
        return false;
    }));
    emscripten::function("VSliderFloat", FUNCTION(bool, (std::string label, emscripten::val size, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::VSliderFloat(label.c_str(), import_ImVec2(size), access_value<float>(v), import_value<float>(v_min), import_value<float>(v_max), import_maybe_null_string(format), flags);
    }));
    emscripten::function("VSliderInt", FUNCTION(bool, (std::string label, emscripten::val size, emscripten::val v, int v_min, int v_max, emscripten::val format, ImGuiSliderFlags flags), {
        return ImGui::VSliderInt(label.c_str(), import_ImVec2(size), access_value<int>(v), v_min, v_max, import_maybe_null_string(format), flags);
    }));
    emscripten::function("VSliderScalar", FUNCTION(bool, (std::string label, emscripten::val size, ImGuiDataType data_type, emscripten::val v, emscripten::val v_min, emscripten::val v_max, emscripten::val format, ImGuiSliderFlags flags), {
        switch (data_type) {
            case ImGuiDataType_S8:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImS8>(v).data(), import_maybe_null_value<ImS8>(v_min), import_maybe_null_value<ImS8>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U8:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImU8>(v).data(), import_maybe_null_value<ImU8>(v_min), import_maybe_null_value<ImU8>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_S16:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImS16>(v).data(), import_maybe_null_value<ImS16>(v_min), import_maybe_null_value<ImS16>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U16:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImU16>(v).data(), import_maybe_null_value<ImU16>(v_min), import_maybe_null_value<ImU16>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_S32:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImS32>(v).data(), import_maybe_null_value<ImS32>(v_min), import_maybe_null_value<ImS32>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_U32:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImU32>(v).data(), import_maybe_null_value<ImU32>(v_min), import_maybe_null_value<ImU32>(v_max), import_maybe_null_string(format), flags);
            // case ImGuiDataType_S64:
            //     return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImS64>(v).data(), import_maybe_null_value<ImS64>(v_min), import_maybe_null_value<ImS64>(v_max), import_maybe_null_string(format), flags);
            // case ImGuiDataType_U64:
            //     return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<ImU64>(v).data(), import_maybe_null_value<ImU64>(v_min), import_maybe_null_value<ImU64>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_Float:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<float>(v).data(), import_maybe_null_value<float>(v_min), import_maybe_null_value<float>(v_max), import_maybe_null_string(format), flags);
            case ImGuiDataType_Double:
                return ImGui::VSliderScalar(label.c_str(), import_ImVec2(size), data_type, access_typed_array<double>(v).data(), import_maybe_null_value<double>(v_min), import_maybe_null_value<double>(v_max), import_maybe_null_string(format), flags);
        }
        return false;
    }));

    // Widgets: Input with Keyboard
    // - If you want to use InputText() with std::string or any custom dynamic string type, see misc/cpp/imgui_stdlib.h and comments in imgui_demo.cpp.
    // - Most of the ImGuiInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX, InputDouble etc.
    // IMGUI_API bool          InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    // IMGUI_API bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    // IMGUI_API bool          InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    // IMGUI_API bool          InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputFloat2(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputFloat3(const char* label, float v[3], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputFloat4(const char* label, float v[4], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);
    // IMGUI_API bool          InputScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);
    emscripten::function("InputText", FUNCTION(bool, (std::string label, emscripten::val buf, size_t buf_size, ImGuiInputTextFlags flags, emscripten::val callback, emscripten::val user_data), {
        std::string _buf = buf[0].as<std::string>();
        _buf.reserve(buf_size);
        bool ret = false;
        if (!callback.isNull()) {
            WrapImGuiContext::GetCurrentContext()->_ImGui_InputText_callback = callback;
            ret = ImGui::InputText(label.c_str(), (char*) _buf.data(), buf_size, flags, FUNCTION(int, (ImGuiInputTextCallbackData* data), {
                return WrapImGuiContext::GetCurrentContext()->_ImGui_InputText_callback(emscripten::val(data)).as<int>();
            }), NULL);
        } else {
            ret = ImGui::InputText(label.c_str(), (char*) _buf.data(), buf_size, flags);
        }
        std::string out_buf (_buf.c_str());
        buf.set(0, out_buf);
        return ret;
    }), emscripten::allow_raw_pointers());
    emscripten::function("InputTextMultiline", FUNCTION(bool, (std::string label, emscripten::val buf, size_t buf_size, emscripten::val size, ImGuiInputTextFlags flags, emscripten::val callback, emscripten::val user_data), {
        std::string _buf = buf[0].as<std::string>();
        _buf.reserve(buf_size);
        bool ret = false;
        if (!callback.isNull()) {
            WrapImGuiContext::GetCurrentContext()->_ImGui_InputTextMultiline_callback = callback;
            ret = ImGui::InputTextMultiline(label.c_str(), (char*) _buf.data(), buf_size, import_ImVec2(size), flags, FUNCTION(int, (ImGuiInputTextCallbackData* data), {
                return WrapImGuiContext::GetCurrentContext()->_ImGui_InputTextMultiline_callback(emscripten::val(data)).as<int>();
            }), NULL);
        } else {
            ret = ImGui::InputTextMultiline(label.c_str(), (char*) _buf.data(), buf_size, import_ImVec2(size), flags);
        }
        std::string out_buf (_buf.c_str());
        buf.set(0, out_buf);
        return ret;
    }));
    emscripten::function("InputTextWithHint", FUNCTION(bool, (std::string label, std::string hint, emscripten::val buf, size_t buf_size, ImGuiInputTextFlags flags, emscripten::val callback, emscripten::val user_data), {
        std::string _buf = buf[0].as<std::string>();
        _buf.reserve(buf_size);
        bool ret = false;
        if (!callback.isNull()) {
            WrapImGuiContext::GetCurrentContext()->_ImGui_InputText_callback = callback;
            ret = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), (char*) _buf.data(), buf_size, flags, FUNCTION(int, (ImGuiInputTextCallbackData* data), {
                return WrapImGuiContext::GetCurrentContext()->_ImGui_InputText_callback(emscripten::val(data)).as<int>();
            }), NULL);
        } else {
            ret = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), (char*) _buf.data(), buf_size, flags);
        }
        std::string out_buf (_buf.c_str());
        buf.set(0, out_buf);
        return ret;
    }), emscripten::allow_raw_pointers());
    emscripten::function("InputFloat", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val step, emscripten::val step_fast, emscripten::val format, ImGuiInputTextFlags flags), {
        return ImGui::InputFloat(label.c_str(), access_value<float>(v), import_value<float>(step), import_value<float>(step_fast), import_maybe_null_string(format), flags);
    }));
    emscripten::function("InputFloat2", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val format, ImGuiInputTextFlags flags), {
        return ImGui::InputFloat2(label.c_str(), access_value<float, 2>(v), import_maybe_null_string(format), flags);
    }));
    emscripten::function("InputFloat3", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val format, ImGuiInputTextFlags flags), {
        return ImGui::InputFloat3(label.c_str(), access_value<float, 3>(v), import_maybe_null_string(format), flags);
    }));
    emscripten::function("InputFloat4", FUNCTION(bool, (std::string label, emscripten::val v, emscripten::val format, ImGuiInputTextFlags flags), {
        return ImGui::InputFloat4(label.c_str(), access_value<float, 4>(v), import_maybe_null_string(format), flags);
    }));
    emscripten::function("InputInt", FUNCTION(bool, (std::string label, emscripten::val v, int step, int step_fast, ImGuiInputTextFlags flags), {
        return ImGui::InputInt(label.c_str(), access_value<int>(v), step, step_fast, flags);
    }));
    emscripten::function("InputInt2", FUNCTION(bool, (std::string label, emscripten::val v, ImGuiInputTextFlags flags), {
        return ImGui::InputInt2(label.c_str(), access_value<int, 2>(v), flags);
    }));
    emscripten::function("InputInt3", FUNCTION(bool, (std::string label, emscripten::val v, ImGuiInputTextFlags flags), {
        return ImGui::InputInt3(label.c_str(), access_value<int, 3>(v), flags);
    }));
    emscripten::function("InputInt4", FUNCTION(bool, (std::string label, emscripten::val v, ImGuiInputTextFlags flags), {
        return ImGui::InputInt4(label.c_str(), access_value<int, 4>(v), flags);
    }));
    emscripten::function("InputDouble", FUNCTION(bool, (std::string label, emscripten::val v, double step, double step_fast, emscripten::val format, ImGuiInputTextFlags flags), {
        return ImGui::InputDouble(label.c_str(), access_value<double>(v), step, step_fast, import_maybe_null_string(format), flags);
    }));
    emscripten::function("InputScalar", FUNCTION(bool, (std::string label, ImGuiDataType data_type, emscripten::val v, emscripten::val step, emscripten::val step_fast, emscripten::val format, ImGuiInputTextFlags flags), {
        switch (data_type) {
            case ImGuiDataType_S8:
                return ImGui::InputScalarV<ImS8>(label.c_str(), data_type, access_typed_array<ImS8>(v), import_maybe_null_value<ImS8>(step), import_maybe_null_value<ImS8>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_U8:
                return ImGui::InputScalarV<ImU8>(label.c_str(), data_type, access_typed_array<ImU8>(v), import_maybe_null_value<ImU8>(step), import_maybe_null_value<ImU8>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_S16:
                return ImGui::InputScalarV<ImS16>(label.c_str(), data_type, access_typed_array<ImS16>(v), import_maybe_null_value<ImS16>(step), import_maybe_null_value<ImS16>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_U16:
                return ImGui::InputScalarV<ImU16>(label.c_str(), data_type, access_typed_array<ImU16>(v), import_maybe_null_value<ImU16>(step), import_maybe_null_value<ImU16>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_S32:
                return ImGui::InputScalarV<ImS32>(label.c_str(), data_type, access_typed_array<ImS32>(v), import_maybe_null_value<ImS32>(step), import_maybe_null_value<ImS32>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_U32:
                return ImGui::InputScalarV<ImU32>(label.c_str(), data_type, access_typed_array<ImU32>(v), import_maybe_null_value<ImU32>(step), import_maybe_null_value<ImU32>(step_fast), import_maybe_null_string(format), flags);
            // case ImGuiDataType_S64:
            //     return ImGui::InputScalarV<ImS64>(label.c_str(), data_type, access_typed_array<ImS64>(v), import_maybe_null_value<ImS64>(step), import_maybe_null_value<ImS64>(step_fast), import_maybe_null_string(format), flags);
            // case ImGuiDataType_U64:
            //     return ImGui::InputScalarV<ImU64>(label.c_str(), data_type, access_typed_array<ImU64>(v), import_maybe_null_value<ImU64>(step), import_maybe_null_value<ImU64>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_Float:
                return ImGui::InputScalarV<float>(label.c_str(), data_type, access_typed_array<float>(v), import_maybe_null_value<float>(step), import_maybe_null_value<float>(step_fast), import_maybe_null_string(format), flags);
            case ImGuiDataType_Double:
                return ImGui::InputScalarV<double>(label.c_str(), data_type, access_typed_array<double>(v), import_maybe_null_value<double>(step), import_maybe_null_value<double>(step_fast), import_maybe_null_string(format), flags);
        }
        return false;
    }));

    // Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
    // - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible.
    // - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
    // IMGUI_API bool          ColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
    // IMGUI_API bool          ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0);
    // IMGUI_API bool          ColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
    // IMGUI_API bool          ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
    // IMGUI_API bool          ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
    // IMGUI_API void          SetColorEditOptions(ImGuiColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.
    emscripten::function("ColorEdit3", FUNCTION(bool, (std::string label, emscripten::val col, ImGuiColorEditFlags flags), { return ImGui::ColorEdit3(label.c_str(), access_value<float, 3>(col), flags); }));
    emscripten::function("ColorEdit4", FUNCTION(bool, (std::string label, emscripten::val col, ImGuiColorEditFlags flags), { return ImGui::ColorEdit4(label.c_str(), access_value<float, 4>(col), flags); }));
    emscripten::function("ColorPicker3", FUNCTION(bool, (std::string label, emscripten::val col, ImGuiColorEditFlags flags), { return ImGui::ColorPicker3(label.c_str(), access_value<float, 3>(col), flags); }));
    emscripten::function("ColorPicker4", FUNCTION(bool, (std::string label, emscripten::val col, ImGuiColorEditFlags flags, emscripten::val ref_col), { return ImGui::ColorPicker4(label.c_str(), access_value<float, 4>(col), flags, access_maybe_null_value<float, 4>(ref_col)); }));
    emscripten::function("ColorButton", FUNCTION(bool, (std::string label, emscripten::val col, ImGuiColorEditFlags flags, emscripten::val size), { return ImGui::ColorButton(label.c_str(), import_ImVec4(col), flags, import_ImVec2(size)); }));
    emscripten::function("SetColorEditOptions", &ImGui::SetColorEditOptions);

    // Widgets: Trees
    // - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
    // IMGUI_API bool          TreeNode(const char* label);
    // IMGUI_API bool          TreeNode(const char* str_id, const char* fmt, ...) IM_FMTARGS(2);   // helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
    // IMGUI_API bool          TreeNode(const void* ptr_id, const char* fmt, ...) IM_FMTARGS(2);   // "
    // IMGUI_API bool          TreeNodeV(const char* str_id, const char* fmt, va_list args) IM_FMTLIST(2);
    // IMGUI_API bool          TreeNodeV(const void* ptr_id, const char* fmt, va_list args) IM_FMTLIST(2);
    // IMGUI_API bool          TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0);
    // IMGUI_API bool          TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(3);
    // IMGUI_API bool          TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(3);
    // IMGUI_API bool          TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(3);
    // IMGUI_API bool          TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(3);
    // IMGUI_API void          TreePush(const char* str_id);                                       // ~ Indent()+PushId(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
    // IMGUI_API void          TreePush(const void* ptr_id = NULL);                                // "
    // IMGUI_API void          TreePop();                                                          // ~ Unindent()+PopId()
    // IMGUI_API float         GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
    // IMGUI_API bool          CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
    // IMGUI_API bool          CollapsingHeader(const char* label, bool* p_visible, ImGuiTreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
    // IMGUI_API void          SetNextItemOpen(bool is_open, ImGuiCond cond = 0);                  // set next TreeNode/CollapsingHeader open state.
    emscripten::function("TreeNode_A", FUNCTION(bool, (std::string label), { return ImGui::TreeNode(label.c_str()); }));
    emscripten::function("TreeNode_B", FUNCTION(bool, (std::string str_id, std::string fmt), { return ImGui::TreeNode(str_id.c_str(), "%s", fmt.c_str()); }));
    emscripten::function("TreeNode_C", FUNCTION(bool, (int ptr_id, std::string fmt), { return ImGui::TreeNode((const void*) ptr_id, "%s", fmt.c_str()); }));
    emscripten::function("TreeNodeEx_A", FUNCTION(bool, (std::string label, ImGuiTreeNodeFlags flags), { return ImGui::TreeNodeEx(label.c_str(), flags); }));
    emscripten::function("TreeNodeEx_B", FUNCTION(bool, (std::string str_id, ImGuiTreeNodeFlags flags, std::string fmt), { return ImGui::TreeNodeEx(str_id.c_str(), flags, "%s", fmt.c_str()); }));
    emscripten::function("TreeNodeEx_C", FUNCTION(bool, (int ptr_id, ImGuiTreeNodeFlags flags, std::string fmt), { return ImGui::TreeNodeEx((const void*) ptr_id, flags, "%s", fmt.c_str());     }));
    emscripten::function("TreePush_A", FUNCTION(void, (std::string str_id), { ImGui::TreePush(str_id.c_str()); }));
    emscripten::function("TreePush_B", FUNCTION(void, (int ptr_id), { ImGui::TreePush((const void*) ptr_id); }));
    emscripten::function("TreePop", &ImGui::TreePop);
    emscripten::function("GetTreeNodeToLabelSpacing", &ImGui::GetTreeNodeToLabelSpacing);
    emscripten::function("CollapsingHeader_A", FUNCTION(bool, (std::string label, ImGuiTreeNodeFlags flags), { return ImGui::CollapsingHeader(label.c_str(), flags); }));
    emscripten::function("CollapsingHeader_B", FUNCTION(bool, (std::string label, emscripten::val p_open, ImGuiTreeNodeFlags flags), { return ImGui::CollapsingHeader(label.c_str(), access_maybe_null_value<bool>(p_open), flags); }));
    emscripten::function("SetNextItemOpen", &ImGui::SetNextItemOpen);
    emscripten::function("SetNextItemStorageID", FUNCTION(void, (unsigned int storage_id), { ImGui::SetNextItemStorageID(storage_id); }));
    emscripten::function("TreeNodeGetOpen", FUNCTION(bool, (unsigned int storage_id), { return ImGui::TreeNodeGetOpen(storage_id); }));
    emscripten::function("TreeNodeSetOpen", FUNCTION(void, (unsigned int storage_id, bool open), { ImGui::TreeNodeSetOpen(storage_id, open); }));

    // Widgets: Selectables
    // - A selectable highlights when hovered, and can display another color when selected.
    // - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.
    // IMGUI_API bool          Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
    // IMGUI_API bool          Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));      // "bool* p_selected" point to the selection state (read-write), as a convenient helper.
    emscripten::function("Selectable_A", FUNCTION(bool, (std::string label, bool selected, ImGuiSelectableFlags flags, emscripten::val size), {
        return ImGui::Selectable(label.c_str(), selected, flags, import_ImVec2(size));    
    }));
    emscripten::function("Selectable_B", FUNCTION(bool, (std::string label, emscripten::val p_selected, ImGuiSelectableFlags flags, emscripten::val size), {
        return ImGui::Selectable(label.c_str(), access_maybe_null_value<bool>(p_selected), flags, import_ImVec2(size));
    }));

    // Multi-Select
    emscripten::function("BeginMultiSelect", FUNCTION(emscripten::val, (ImGuiMultiSelectFlags flags, int selection_size, int items_count), {
        ImGuiMultiSelectIO* p = ImGui::BeginMultiSelect(flags, selection_size, items_count);
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    emscripten::function("EndMultiSelect", FUNCTION(emscripten::val, (), {
        ImGuiMultiSelectIO* p = ImGui::EndMultiSelect();
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    emscripten::function("SetNextItemSelectionUserData", FUNCTION(void, (double selection_user_data), { ImGui::SetNextItemSelectionUserData((ImGuiSelectionUserData)selection_user_data); }));
    emscripten::function("IsItemToggledSelection", &ImGui::IsItemToggledSelection);

    // Widgets: List Boxes
    // - FIXME: To be consistent with all the newer API, ListBoxHeader/ListBoxFooter should in reality be called BeginListBox/EndListBox. Will rename them.
    // IMGUI_API bool          ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
    // IMGUI_API bool          ListBox(const char* label, int* current_item, bool (*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items = -1);
    // IMGUI_API bool          ListBoxHeader(const char* label, const ImVec2& size = ImVec2(0, 0)); // use if you want to reimplement ListBox() will custom data or interactions. if the function return true, you can output elements then call ListBoxFooter() afterwards.
    // IMGUI_API bool          ListBoxHeader(const char* label, int items_count, int height_in_items = -1); // "
    // IMGUI_API void          ListBoxFooter();                                                    // terminate the scrolling region. only call ListBoxFooter() if ListBoxHeader() returned true!
    emscripten::function("ListBox_A", FUNCTION(bool, (std::string label, emscripten::val current_item, emscripten::val items, int items_count, int height_in_items), {
        WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
        ctx->_ImGui_ListBox_A_items = items;
        ctx->_ImGui_ListBox_A_items_count = items_count;
        return ImGui::ListBox(label.c_str(), access_value<int>(current_item), FUNCTION(const char*, (void* user_data, int idx), {
            WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
            if (0 <= idx && idx < ctx->_ImGui_ListBox_A_items_count) {
                ctx->_ImGui_ListBox_A_text = ctx->_ImGui_ListBox_A_items[idx].as<std::string>();
                return ctx->_ImGui_ListBox_A_text.c_str();
            } else {
                return (const char*)NULL;
            }
        }), NULL, items_count, height_in_items);
    }));
    emscripten::function("ListBox_B", FUNCTION(bool, (std::string label, emscripten::val current_item, emscripten::val items_getter, emscripten::val data, int items_count, int height_in_items), {
        WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
        ctx->_ImGui_ListBox_B_items_getter = items_getter;
        ctx->_ImGui_ListBox_B_data = data;
        ctx->_ImGui_ListBox_B_items_count = items_count;
        return ImGui::ListBox(label.c_str(), access_value<int>(current_item), FUNCTION(const char*, (void* user_data, int idx), {
            WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
            if (0 <= idx && idx < ctx->_ImGui_ListBox_B_items_count) {
                emscripten::val ret = ctx->_ImGui_ListBox_B_items_getter(ctx->_ImGui_ListBox_B_data, emscripten::val(idx));
                ctx->_ImGui_ListBox_B_text = ret.as<std::string>();
                return ctx->_ImGui_ListBox_B_text.c_str();
            } else {
                return (const char*)NULL;
            }
        }), NULL, items_count, height_in_items);
    }));
    emscripten::function("BeginListBox", FUNCTION(bool, (std::string label, emscripten::val size), {
        return ImGui::BeginListBox(label.c_str(), import_ImVec2(size));
    }));
    emscripten::function("EndListBox", &ImGui::EndListBox);

    // Widgets: Data Plotting
    // IMGUI_API void          PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
    // IMGUI_API void          PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
    // IMGUI_API void          PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
    // IMGUI_API void          PlotHistogram(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
    emscripten::function("PlotLines", FUNCTION(void, (std::string label, emscripten::val values_getter, emscripten::val data, int values_count, int values_offset, emscripten::val overlay_text, emscripten::val scale_min, emscripten::val scale_max, emscripten::val graph_size), {
        WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
        ctx->_ImGui_PlotLines_values_getter = values_getter;
        ctx->_ImGui_PlotLines_data = data;
        ImGui::PlotLines(label.c_str(), FUNCTION(float, (void* data, int idx), {
            WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
            return import_value<float>(ctx->_ImGui_PlotLines_values_getter(ctx->_ImGui_PlotLines_data, emscripten::val(idx)));
        }), NULL, values_count, values_offset, import_maybe_null_string(overlay_text), import_value<float>(scale_min), import_value<float>(scale_max), import_ImVec2(graph_size));
    }));
    emscripten::function("PlotHistogram", FUNCTION(void, (std::string label, emscripten::val values_getter, emscripten::val data, int values_count, int values_offset, emscripten::val overlay_text, emscripten::val scale_min, emscripten::val scale_max, emscripten::val graph_size), {
        WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
        ctx->_ImGui_PlotHistogram_values_getter = values_getter;
        ctx->_ImGui_PlotHistogram_data = data;
        ImGui::PlotHistogram(label.c_str(), FUNCTION(float, (void* data, int idx), {
            WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
            return import_value<float>(ctx->_ImGui_PlotHistogram_values_getter(ctx->_ImGui_PlotHistogram_data, emscripten::val(idx)));
        }), NULL, values_count, values_offset, import_maybe_null_string(overlay_text), import_value<float>(scale_min), import_value<float>(scale_max), import_ImVec2(graph_size));
    }));

    // Widgets: Value() Helpers.
    // - Those are merely shortcut to calling Text() with a format string. Output single value in "name: value" format (tip: freely declare more in your code to handle your types. you can add functions to the ImGui namespace)
    // IMGUI_API void          Value(const char* prefix, bool b);
    // IMGUI_API void          Value(const char* prefix, int v);
    // IMGUI_API void          Value(const char* prefix, unsigned int v);
    // IMGUI_API void          Value(const char* prefix, float v, const char* float_format = NULL);
    emscripten::function("Value_A", FUNCTION(void, (std::string prefix, bool b), { ImGui::Value(prefix.c_str(), b); }));
    emscripten::function("Value_B", FUNCTION(void, (std::string prefix, int v), { ImGui::Value(prefix.c_str(), v); }));
    emscripten::function("Value_C", FUNCTION(void, (std::string prefix, unsigned int v), { ImGui::Value(prefix.c_str(), v); }));
    emscripten::function("Value_D", FUNCTION(void, (std::string prefix, float v, emscripten::val float_format), { ImGui::Value(prefix.c_str(), v, import_maybe_null_string(float_format)); }));

    // Widgets: Menus
    // - Use BeginMenuBar() on a window ImGuiWindowFlags_MenuBar to append to its menu bar.
    // - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
    // - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more items to it.
    // IMGUI_API bool          BeginMenuBar();                                                     // append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on parent window).
    // IMGUI_API void          EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
    // IMGUI_API bool          BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
    // IMGUI_API void          EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
    // IMGUI_API bool          BeginMenu(const char* label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
    // IMGUI_API void          EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
    // IMGUI_API bool          MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated. shortcuts are displayed for convenience but not processed by ImGui at the moment
    // IMGUI_API bool          MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL
    emscripten::function("BeginMenuBar", &ImGui::BeginMenuBar);
    emscripten::function("EndMenuBar", &ImGui::EndMenuBar);
    emscripten::function("BeginMainMenuBar", &ImGui::BeginMainMenuBar);
    emscripten::function("EndMainMenuBar", &ImGui::EndMainMenuBar);
    emscripten::function("BeginMenu", FUNCTION(bool, (std::string label, bool enabled), { return ImGui::BeginMenu(label.c_str(), enabled); }));
    emscripten::function("EndMenu", &ImGui::EndMenu);
    emscripten::function("MenuItem_A", FUNCTION(bool, (std::string label, emscripten::val shortcut, bool selected, bool enabled), { return ImGui::MenuItem(label.c_str(), import_maybe_null_string(shortcut), selected, enabled); }));
    emscripten::function("MenuItem_B", FUNCTION(bool, (std::string label, emscripten::val shortcut, emscripten::val p_selected, bool enabled), { return ImGui::MenuItem(label.c_str(), import_maybe_null_string(shortcut), access_maybe_null_value<bool>(p_selected), enabled); }));

    // Tooltips
    // - Tooltip are windows following the mouse. They do not take focus away.
    // IMGUI_API void          BeginTooltip();                                                     // begin/append a tooltip window. to create full-featured tooltip (with any kind of items).
    // IMGUI_API void          EndTooltip();
    // IMGUI_API void          SetTooltip(const char* fmt, ...) IM_FMTARGS(1);                     // set a text-only tooltip, typically use with ImGui::IsItemHovered(). override any previous call to SetTooltip().
    // IMGUI_API void          SetTooltipV(const char* fmt, va_list args) IM_FMTLIST(1);
    emscripten::function("BeginTooltip", &ImGui::BeginTooltip);
    emscripten::function("EndTooltip", &ImGui::EndTooltip);
    emscripten::function("SetTooltip", FUNCTION(void, (std::string fmt), { ImGui::SetTooltip("%s", fmt.c_str()); }));
    emscripten::function("BeginItemTooltip", &ImGui::BeginItemTooltip);
    emscripten::function("SetItemTooltip", FUNCTION(void, (std::string fmt), { ImGui::SetItemTooltip("%s", fmt.c_str()); }));

    // Disable
    emscripten::function("BeginDisabled", &ImGui::BeginDisabled);
    emscripten::function("EndDisabled", &ImGui::EndDisabled);

    // Popups, Modals
    //  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
    //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    //  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
    //  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
    //  - You can bypass the hovering restriction by using ImGuiHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
    //  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
    //    This is sometimes leading to confusing mistakes. May rework this in the future.
    // Popups: begin/end functions
    //  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards. ImGuiWindowFlags are forwarded to the window.
    //  - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar.
    // IMGUI_API bool          BeginPopup(const char* str_id, ImGuiWindowFlags flags = 0);                         // return true if the popup is open, and you can start outputting to it.
    // IMGUI_API bool          BeginPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
    // IMGUI_API void          EndPopup();                                                                         // only call EndPopup() if BeginPopupXXX() returns true!
    emscripten::function("BeginPopup", FUNCTION(bool, (std::string str_id, ImGuiWindowFlags flags), { return ImGui::BeginPopup(str_id.c_str(), flags); }));
    emscripten::function("BeginPopupModal", FUNCTION(bool, (std::string name, emscripten::val p_open, ImGuiWindowFlags flags), { return ImGui::BeginPopupModal(name.c_str(), access_maybe_null_value<bool>(p_open), flags); }));
    emscripten::function("EndPopup", &ImGui::EndPopup);
    // Popups: open/close functions
    //  - OpenPopup(): set popup state to open. ImGuiPopupFlags are available for opening options.
    //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    //  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
    //  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
    //  - Use ImGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
    // IMGUI_API void          OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags = 0);                     // call to mark popup as open (don't call every frame!).
    // IMGUI_API void          OpenPopupOnItemClick(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);   // helper to open popup when clicked on last item. return true when just opened. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
    // IMGUI_API void          CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.
    emscripten::function("OpenPopup", FUNCTION(void, (std::string str_id, ImGuiPopupFlags popup_flags), { ImGui::OpenPopup(str_id.c_str(), popup_flags); }));
    emscripten::function("OpenPopupOnItemClick", FUNCTION(void, (emscripten::val str_id, ImGuiPopupFlags popup_flags), { ImGui::OpenPopupOnItemClick(import_maybe_null_string(str_id), popup_flags); }));
    emscripten::function("CloseCurrentPopup", &ImGui::CloseCurrentPopup);
    // Popups: open+begin combined functions helpers
    //  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
    //  - They are convenient to easily create context menus, hence the name.
    //  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
    //  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.
    // IMGUI_API bool          BeginPopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked on last item. if you can pass a NULL str_id only if the previous item had an id. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
    // IMGUI_API bool          BeginPopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);// open+begin popup when clicked on current window.
    // IMGUI_API bool          BeginPopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked in void (where there are no windows).
    emscripten::function("BeginPopupContextItem", FUNCTION(bool, (emscripten::val str_id, ImGuiPopupFlags popup_flags), { return ImGui::BeginPopupContextItem(import_maybe_null_string(str_id), popup_flags); }));
    emscripten::function("BeginPopupContextWindow", FUNCTION(bool, (emscripten::val str_id, ImGuiPopupFlags popup_flags), { return ImGui::BeginPopupContextWindow(import_maybe_null_string(str_id), popup_flags); }));
    emscripten::function("BeginPopupContextVoid", FUNCTION(bool, (emscripten::val str_id, ImGuiPopupFlags popup_flags), { return ImGui::BeginPopupContextVoid(import_maybe_null_string(str_id), popup_flags); }));
    // Popups: test function
    //  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
    //  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
    //  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
    // IMGUI_API bool          IsPopupOpen(const char* str_id, ImGuiPopupFlags flags = 0);                         // return true if the popup is open.
    emscripten::function("IsPopupOpen", FUNCTION(bool, (std::string str_id, ImGuiPopupFlags flags), { return ImGui::IsPopupOpen(str_id.c_str(), flags); }));

    // Tables
    // [BETA API] API may evolve slightly! If you use this, please update to the next version when it comes out!
    // - Full-featured replacement for old Columns API.
    // - See Demo->Tables for demo code.
    // - See top of imgui_tables.cpp for general commentary.
    // - See ImGuiTableFlags_ and ImGuiTableColumnFlags_ enums for a description of available flags.
    // The typical call flow is:
    // - 1. Call BeginTable().
    // - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
    // - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
    // - 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from TableSetupColumn() data.
    // - 5. Populate contents:
    //    - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending into a column.
    //    - If you are using tables as a sort of grid, where every columns is holding the same type of contents,
    //      you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
    //      TableNextColumn() will automatically wrap-around into the next row if needed.
    //    - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for the first column!
    //    - Summary of possible call flow:
    //        --------------------------------------------------------------------------------------------------------
    //        TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
    //        TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
    //                          TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
    //        TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
    //        --------------------------------------------------------------------------------------------------------
    // - 5. Call EndTable()
    // IMGUI_API bool          BeginTable(const char* str_id, int column, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
    // IMGUI_API void          EndTable();                                 // only call EndTable() if BeginTable() returns true!
    // IMGUI_API void          TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row.
    // IMGUI_API bool          TableNextColumn();                          // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
    // IMGUI_API bool          TableSetColumnIndex(int column_n);          // append into the specified column. Return true when column is visible.
    emscripten::function("BeginTable", FUNCTION(bool, (std::string str_id, int column, ImGuiTableFlags flags, emscripten::val outer_size, float inner_width), { return ImGui::BeginTable(str_id.c_str(), column, flags, import_ImVec2(outer_size), inner_width); }));
    emscripten::function("EndTable", FUNCTION(void, (), { ImGui::EndTable(); }));
    emscripten::function("TableNextRow", FUNCTION(void, (ImGuiTableRowFlags row_flags, float min_row_height), { ImGui::TableNextRow(row_flags, min_row_height); }));
    emscripten::function("TableNextColumn", FUNCTION(bool, (), { return ImGui::TableNextColumn(); }));
    emscripten::function("TableSetColumnIndex", FUNCTION(bool, (int column_n), { return ImGui::TableSetColumnIndex(column_n); }));
    // Tables: Headers & Columns declaration
    // - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
    // - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
    //   Headers are required to perform: reordering, sorting, and opening the context menu.
    //   The context menu can also be made available in columns body using ImGuiTableFlags_ContextMenuInBody.
    // - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
    //   some advanced use cases (e.g. adding custom widgets in header row).
    // - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
    // IMGUI_API void          TableSetupColumn(const char* label, ImGuiTableColumnFlags flags = 0, float init_width_or_weight = 0.0f, ImU32 user_id = 0);
    // IMGUI_API void          TableSetupScrollFreeze(int cols, int rows); // lock columns/rows so they stay visible when scrolled.
    // IMGUI_API void          TableHeadersRow();                          // submit all headers cells based on data provided to TableSetupColumn() + submit context menu
    // IMGUI_API void          TableHeader(const char* label);             // submit one header cell manually (rarely used)
    emscripten::function("TableSetupColumn", FUNCTION(void, (std::string label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImU32 user_id), { ImGui::TableSetupColumn(label.c_str(), flags, init_width_or_weight, user_id); }));
    emscripten::function("TableSetupScrollFreeze", FUNCTION(void, (int cols, int rows), { ImGui::TableSetupScrollFreeze(cols, rows); }));
    emscripten::function("TableHeadersRow", FUNCTION(void, (), { ImGui::TableHeadersRow(); }));
    emscripten::function("TableAngledHeadersRow", FUNCTION(void, (), { ImGui::TableAngledHeadersRow(); }));
    emscripten::function("TableHeader", FUNCTION(void, (std::string label), { ImGui::TableHeader(label.c_str()); }));
    // Tables: Sorting
    // - Call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
    // - When 'SpecsDirty == true' you should sort your data. It will be true when sorting specs have changed
    //   since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting, else you may
    //   wastefully sort your data every frame!
    // - Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
    // IMGUI_API ImGuiTableSortSpecs* TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).
    emscripten::function("TableGetSortSpecs", FUNCTION(emscripten::val, (), { ImGuiTableSortSpecs* p = ImGui::TableGetSortSpecs(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    // Tables: Miscellaneous functions
    // - Functions args 'int column_n' treat the default value of -1 as the same as passing the current column index.
    // IMGUI_API int                   TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
    // IMGUI_API int                   TableGetColumnIndex();                      // return current column index.
    // IMGUI_API int                   TableGetRowIndex();                         // return current row index.
    // IMGUI_API const char*           TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
    // IMGUI_API ImGuiTableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
    // IMGUI_API void                  TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1);  // change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.
    emscripten::function("TableGetColumnCount", FUNCTION(int, (), { return ImGui::TableGetColumnCount(); }));
    emscripten::function("TableGetColumnIndex", FUNCTION(int, (), { return ImGui::TableGetColumnIndex(); }));
    emscripten::function("TableGetRowIndex", FUNCTION(int, (), { return ImGui::TableGetRowIndex(); }));
    emscripten::function("TableGetColumnName", FUNCTION(std::string, (int column_n), { return ImGui::TableGetColumnName(column_n); }));
    emscripten::function("TableGetColumnFlags", FUNCTION(ImGuiTableColumnFlags, (int column_n), { return ImGui::TableGetColumnFlags(column_n); }));
    emscripten::function("TableSetColumnEnabled", FUNCTION(void, (int column_n, bool enabled), { ImGui::TableSetColumnEnabled(column_n, enabled); }));
    emscripten::function("TableGetHoveredColumn", FUNCTION(int, (), { return ImGui::TableGetHoveredColumn(); }));
    emscripten::function("TableSetBgColor", FUNCTION(void, (ImGuiTableBgTarget target, ImU32 color, int column_n), { ImGui::TableSetBgColor(target, color, column_n); }));

    // Legacy Columns API (2020: prefer using Tables!)
    // - You can also use SameLine(pos_x) to mimic simplified columns.
    // IMGUI_API void          Columns(int count = 1, const char* id = NULL, bool border = true);
    // IMGUI_API void          NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
    // IMGUI_API int           GetColumnIndex();                                                   // get current column index
    // IMGUI_API float         GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
    // IMGUI_API void          SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
    // IMGUI_API float         GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
    // IMGUI_API void          SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
    // IMGUI_API int           GetColumnsCount();
    emscripten::function("Columns", FUNCTION(void, (int count, emscripten::val id, bool border), { ImGui::Columns(count, import_maybe_null_string(id), border); }));
    emscripten::function("NextColumn", &ImGui::NextColumn);
    emscripten::function("GetColumnIndex", &ImGui::GetColumnIndex);
    emscripten::function("GetColumnWidth", &ImGui::GetColumnWidth);
    emscripten::function("SetColumnWidth", &ImGui::SetColumnWidth);
    emscripten::function("GetColumnOffset", &ImGui::GetColumnOffset);
    emscripten::function("SetColumnOffset", &ImGui::SetColumnOffset);
    emscripten::function("GetColumnsCount", &ImGui::GetColumnsCount);

    // Tab Bars, Tabs
    // IMGUI_API bool          BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);        // create and append into a TabBar
    // IMGUI_API void          EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
    // IMGUI_API bool          BeginTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
    // IMGUI_API void          EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
    // IMGUI_API bool          TabItemButton(const char* label, ImGuiTabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
    // IMGUI_API void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.
    emscripten::function("BeginTabBar", FUNCTION(bool, (std::string str_id, ImGuiTabBarFlags flags), { return ImGui::BeginTabBar(str_id.c_str(), flags); }));
    emscripten::function("EndTabBar", &ImGui::EndTabBar);
    emscripten::function("BeginTabItem", FUNCTION(bool, (std::string label, emscripten::val p_open, ImGuiTabItemFlags flags), { return ImGui::BeginTabItem(label.c_str(), access_maybe_null_value<bool>(p_open), flags); }));
    emscripten::function("EndTabItem", &ImGui::EndTabItem);
    emscripten::function("TabItemButton", FUNCTION(bool, (std::string label, ImGuiTabItemFlags flags), { return ImGui::TabItemButton(label.c_str(), flags); }));
    emscripten::function("SetTabItemClosed", FUNCTION(void, (std::string tab_or_docked_window_label), { ImGui::SetTabItemClosed(tab_or_docked_window_label.c_str()); }));

    // Docking
    // [BETA API] Enable with io.ConfigFlags |= ImGuiConfigFlags_DockingEnable.
    // Note: You can use most Docking facilities without calling any API. You DO NOT need to call DockSpace() to use Docking!
    // - To dock windows: if io.ConfigDockingWithShift == false (default) drag window from their title bar.
    // - To dock windows: if io.ConfigDockingWithShift == true: hold SHIFT anywhere while moving windows.
    // About DockSpace:
    // - Use DockSpace() to create an explicit dock node _within_ an existing window. See Docking demo for details.
    // - DockSpace() needs to be submitted _before_ any window they can host. If you use a dockspace, submit it early in your app.
    // IMGUI_API void          DockSpace(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiDockNodeFlags flags = 0, const ImGuiWindowClass* window_class = NULL);
    emscripten::function("DockSpace", FUNCTION(void, (emscripten::val id, emscripten::val size, ImGuiDockNodeFlags flags, emscripten::val window_class), {
        if (window_class.isNull()) {
            ImGui::DockSpace(id.as<ImGuiID>(), import_ImVec2(size), flags, NULL);
        } else {
            ImGuiWindowClass wc = import_ImGuiWindowClass(window_class);
            ImGui::DockSpace(id.as<ImGuiID>(), import_ImVec2(size), flags, &wc);
        }
    }));
    emscripten::function("DockSpaceOverViewport", FUNCTION(ImGuiID, (ImGuiID dockspace_id, emscripten::val viewport, ImGuiDockNodeFlags flags, emscripten::val window_class), {
        const ImGuiViewport* _viewport = viewport.isNull() ? NULL : viewport.as<ImGuiViewport*>(emscripten::allow_raw_pointers());
        if (window_class.isNull()) {
            return ImGui::DockSpaceOverViewport(dockspace_id, _viewport, flags, NULL);
        } else {
            ImGuiWindowClass wc = import_ImGuiWindowClass(window_class);
            return ImGui::DockSpaceOverViewport(dockspace_id, _viewport, flags, &wc);
        }
    }), emscripten::allow_raw_pointers());
    // IMGUI_API ImGuiID       DockSpaceOverViewport(ImGuiID dockspace_id = 0, const ImGuiViewport* viewport = NULL, ImGuiDockNodeFlags flags = 0, const ImGuiWindowClass* window_class = NULL);
    emscripten::function("DockSpaceOverMainViewport", FUNCTION(ImGuiID, (ImGuiDockNodeFlags flags), {
          return ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), flags);
    }));
    emscripten::function("DockSpaceOverViewportID", FUNCTION(ImGuiID, (ImGuiID dockspace_id, ImGuiDockNodeFlags flags), {
          return ImGui::DockSpaceOverViewport(dockspace_id, NULL, flags);
    }));
    // IMGUI_API void          SetNextWindowDockID(ImGuiID dock_id, ImGuiCond cond = 0);           // set next window dock id (FIXME-DOCK)
    emscripten::function("SetNextWindowDockID", &ImGui::SetNextWindowDockID);
    // IMGUI_API void          SetNextWindowClass(const ImGuiWindowClass* window_class);           // set next window class (rare/advanced uses: provide hints to the platform backend via altered viewport flags and parent/child info)
    emscripten::function("SetNextWindowClass", FUNCTION(void, (emscripten::val window_class), {
        ImGuiWindowClass wc = import_ImGuiWindowClass(window_class);
        ImGui::SetNextWindowClass(&wc);
    }));
    // IMGUI_API ImGuiID       GetWindowDockID();
    emscripten::function("GetWindowDockID", &ImGui::GetWindowDockID);
    // IMGUI_API bool          IsWindowDocked();                                                   // is current window docked into another window?
    emscripten::function("IsWindowDocked", &ImGui::IsWindowDocked);

    // Logging/Capture
    // - All text output from the interface can be captured into tty/file/clipboard. By default, tree nodes are automatically opened during logging.
    // IMGUI_API void          LogToTTY(int auto_open_depth = -1);                                 // start logging to tty (stdout)
    // IMGUI_API void          LogToFile(int auto_open_depth = -1, const char* filename = NULL);   // start logging to file
    // IMGUI_API void          LogToClipboard(int auto_open_depth = -1);                           // start logging to OS clipboard
    // IMGUI_API void          LogFinish();                                                        // stop logging (close file, etc.)
    // IMGUI_API void          LogButtons();                                                       // helper to display buttons for logging to tty/file/clipboard
    // IMGUI_API void          LogText(const char* fmt, ...) IM_FMTARGS(1);                        // pass text data straight to log (without being displayed)
    emscripten::function("LogToTTY", &ImGui::LogToTTY);
    emscripten::function("LogToFile", FUNCTION(void, (int max_depth, emscripten::val filename), { ImGui::LogToFile(max_depth, import_maybe_null_string(filename)); }));
    emscripten::function("LogToClipboard", &ImGui::LogToClipboard);
    emscripten::function("LogFinish", &ImGui::LogFinish);
    emscripten::function("LogButtons", &ImGui::LogButtons);
    emscripten::function("LogText", FUNCTION(void, (std::string fmt), { ImGui::LogText("%s", fmt.c_str()); }));

    // Drag and Drop
    // - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip as replacement)
    // IMGUI_API bool          BeginDragDropSource(ImGuiDragDropFlags flags = 0);                                      // call when the current item is active. If this return true, you can call SetDragDropPayload() + EndDragDropSource()
    // IMGUI_API bool          SetDragDropPayload(const char* type, const void* data, size_t sz, ImGuiCond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui.
    // IMGUI_API void          EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
    // IMGUI_API bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
    // IMGUI_API const ImGuiPayload*   AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags = 0);          // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
    // IMGUI_API void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
    // IMGUI_API const ImGuiPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.
    emscripten::function("BeginDragDropSource", &ImGui::BeginDragDropSource);
    emscripten::function("SetDragDropPayload", FUNCTION(bool, (std::string type, emscripten::val data, ImGuiCond cond), {
        g_DragDropPayloadData = data;
        // Pass a small marker to ImGui so it tracks the payload lifecycle
        int marker = 1;
        return ImGui::SetDragDropPayload(type.c_str(), &marker, sizeof(marker), cond);
    }));
    emscripten::function("EndDragDropSource", &ImGui::EndDragDropSource);
    emscripten::function("BeginDragDropTarget", &ImGui::BeginDragDropTarget);
    emscripten::function("AcceptDragDropPayload", FUNCTION(emscripten::val, (std::string type, ImGuiDragDropFlags flags), {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type.c_str(), flags);
        if (payload == NULL) { return emscripten::val::null(); }
        return g_DragDropPayloadData;
    }));
    emscripten::function("EndDragDropTarget", &ImGui::EndDragDropTarget);
    emscripten::function("GetDragDropPayload", FUNCTION(emscripten::val, (), {
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        if (payload == NULL) { return emscripten::val::null(); }
        return g_DragDropPayloadData;
    }));

    // Clipping
    // - Mouse hovering is affected by ImGui::PushClipRect() calls, unlike direct calls to ImDrawList::PushClipRect() which are render only.
    // IMGUI_API void          PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect);
    // IMGUI_API void          PopClipRect();
    emscripten::function("PushClipRect", FUNCTION(void, (emscripten::val clip_rect_min, emscripten::val clip_rect_max, bool intersect_with_current_clip_rect), {
        return ImGui::PushClipRect(import_ImVec2(clip_rect_min), import_ImVec2(clip_rect_max), intersect_with_current_clip_rect);
    }));
    emscripten::function("PopClipRect", &ImGui::PopClipRect);

    // Focus, Activation
    // - Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify "this is the default item"
    // IMGUI_API void          SetItemDefaultFocus();                                              // make last item the default focused item of a window.
    // IMGUI_API void          SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.
    emscripten::function("SetItemDefaultFocus", &ImGui::SetItemDefaultFocus);
    emscripten::function("SetKeyboardFocusHere", &ImGui::SetKeyboardFocusHere);

    // Item/Widgets Utilities
    // - Most of the functions are referring to the last/previous item we submitted.
    // - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
    // IMGUI_API bool          IsItemHovered(ImGuiHoveredFlags flags = 0);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.
    // IMGUI_API bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
    // IMGUI_API bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?
    // IMGUI_API bool          IsItemClicked(ImGuiMouseButton mouse_button = 0);                   // is the last item clicked? (e.g. button/node just clicked on) == IsMouseClicked(mouse_button) && IsItemHovered()
    // IMGUI_API bool          IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
    // IMGUI_API bool          IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
    // IMGUI_API bool          IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
    // IMGUI_API bool          IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that requires continuous editing.
    // IMGUI_API bool          IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that requires continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
    // IMGUI_API bool          IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
    // IMGUI_API bool          IsAnyItemHovered();                                                 // is any item hovered?
    // IMGUI_API bool          IsAnyItemActive();                                                  // is any item active?
    // IMGUI_API bool          IsAnyItemFocused();                                                 // is any item focused?
    // IMGUI_API ImVec2        GetItemRectMin();                                                   // get upper-left bounding rectangle of the last item (screen space)
    // IMGUI_API ImVec2        GetItemRectMax();                                                   // get lower-right bounding rectangle of the last item (screen space)
    // IMGUI_API ImVec2        GetItemRectSize();                                                  // get size of last item
    // IMGUI_API void          SetItemAllowOverlap();                                              // allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.
    emscripten::function("IsItemHovered", &ImGui::IsItemHovered);
    emscripten::function("IsItemActive", &ImGui::IsItemActive);
    emscripten::function("IsItemFocused", &ImGui::IsItemFocused);
    emscripten::function("IsItemClicked", &ImGui::IsItemClicked);
    emscripten::function("IsItemVisible", &ImGui::IsItemVisible);
    emscripten::function("IsItemEdited", &ImGui::IsItemEdited);
    emscripten::function("IsItemActivated", &ImGui::IsItemActivated);
    emscripten::function("IsItemDeactivated", &ImGui::IsItemDeactivated);
    emscripten::function("IsItemDeactivatedAfterEdit", &ImGui::IsItemDeactivatedAfterEdit);
    emscripten::function("IsItemToggledOpen", &ImGui::IsItemToggledOpen);
    emscripten::function("IsAnyItemHovered", &ImGui::IsAnyItemHovered);
    emscripten::function("IsAnyItemActive", &ImGui::IsAnyItemActive);
    emscripten::function("IsAnyItemFocused", &ImGui::IsAnyItemFocused);
    emscripten::function("GetItemRectMin", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetItemRectMin(), out); }));
    emscripten::function("GetItemRectMax", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetItemRectMax(), out); }));
    emscripten::function("GetItemRectSize", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetItemRectSize(), out); }));
    emscripten::function("SetNextItemAllowOverlap", &ImGui::SetNextItemAllowOverlap);
    emscripten::function("SetItemAllowOverlap", &ImGui::SetNextItemAllowOverlap);

    // Miscellaneous Utilities
    // IMGUI_API bool          IsRectVisible(const ImVec2& size);                                  // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
    // IMGUI_API bool          IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max);      // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
    // IMGUI_API double        GetTime();                                                          // get global imgui time. incremented by io.DeltaTime every frame.
    // IMGUI_API int           GetFrameCount();                                                    // get global imgui frame count. incremented by 1 every frame.
    // IMGUI_API ImDrawList*   GetBackgroundDrawList();                                            // this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.
    // IMGUI_API ImDrawList*   GetForegroundDrawList();                                            // this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.
    // IMGUI_API ImDrawListSharedData* GetDrawListSharedData();                                    // you may use this when creating your own ImDrawList instances.
    // IMGUI_API const char*   GetStyleColorName(ImGuiCol idx);                                    // get a string corresponding to the enum value (for display, saving, etc.).
    // IMGUI_API void          SetStateStorage(ImGuiStorage* storage);                             // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
    // IMGUI_API ImGuiStorage* GetStateStorage();
    // IMGUI_API void          CalcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end);    // calculate coarse clipping for large list of evenly sized items. Prefer using the ImGuiListClipper higher-level helper if you can.
    // IMGUI_API bool          BeginChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags flags = 0); // helper to create a child window / scrolling region that looks like a normal widget frame
    // IMGUI_API void          EndChildFrame();                                                    // always call EndChildFrame() regardless of BeginChildFrame() return values (which indicates a collapsed/clipped window)
    emscripten::function("IsRectVisible_A", FUNCTION(bool, (emscripten::val size), { return ImGui::IsRectVisible(import_ImVec2(size)); }));
    emscripten::function("IsRectVisible_B", FUNCTION(bool, (emscripten::val rect_min, emscripten::val rect_max), { return ImGui::IsRectVisible(import_ImVec2(rect_min), import_ImVec2(rect_max)); }));
    emscripten::function("GetTime", &ImGui::GetTime);
    emscripten::function("GetFrameCount", &ImGui::GetFrameCount);
    emscripten::function("GetBackgroundDrawList", FUNCTION(emscripten::val, (), { ImDrawList* p = ImGui::GetBackgroundDrawList(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("GetForegroundDrawList", FUNCTION(emscripten::val, (), { ImDrawList* p = ImGui::GetForegroundDrawList(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("GetDrawListSharedData", FUNCTION(emscripten::val, (), { ImDrawListSharedData* p = ImGui::GetDrawListSharedData(); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("GetStyleColorName", FUNCTION(std::string, (ImGuiCol idx), { return std::string(ImGui::GetStyleColorName(idx)); }));
    emscripten::function("SetStateStorage", FUNCTION(void, (emscripten::val storage), {
        ImGui::SetStateStorage(storage.isNull() ? NULL : storage.as<ImGuiStorage*>(emscripten::allow_raw_pointers()));
    }), emscripten::allow_raw_pointers());
    emscripten::function("GetStateStorage", FUNCTION(emscripten::val, (), {
        ImGuiStorage* p = ImGui::GetStateStorage();
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    // CalcListClipping removed in v1.92 - use ImGuiListClipper instead
    // BeginChildFrame/EndChildFrame removed in v1.92 - use BeginChild with ImGuiChildFlags_FrameStyle instead

    // Text Utilities
    // IMGUI_API ImVec2        CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
    emscripten::function("CalcTextSize", FUNCTION(emscripten::val, (std::string text, bool hide_text_after_double_hash, float wrap_width, emscripten::val out), {
        return export_ImVec2(ImGui::CalcTextSize(text.c_str(), NULL, hide_text_after_double_hash, wrap_width), out);
    }));

    // Color Utilities
    // IMGUI_API ImVec4        ColorConvertU32ToFloat4(ImU32 in);
    // IMGUI_API ImU32         ColorConvertFloat4ToU32(const ImVec4& in);
    // IMGUI_API void          ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
    // IMGUI_API void          ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);
    emscripten::function("ColorConvertU32ToFloat4", FUNCTION(emscripten::val, (ImU32 in, emscripten::val out), { return export_ImVec4(ImGui::ColorConvertU32ToFloat4(in), out); }));
    emscripten::function("ColorConvertFloat4ToU32", FUNCTION(ImU32, (emscripten::val in), { return ImGui::ColorConvertFloat4ToU32(import_ImVec4(in)); }));
    emscripten::function("ColorConvertRGBtoHSV", FUNCTION(void, (float r, float g, float b, emscripten::val out_h, emscripten::val out_s, emscripten::val out_v), {
        ImGui::ColorConvertRGBtoHSV(r, g, b, access_value<float>(out_h), access_value<float>(out_s), access_value<float>(out_v));
    }));
    emscripten::function("ColorConvertHSVtoRGB", FUNCTION(void, (float h, float s, float v, emscripten::val out_r, emscripten::val out_g, emscripten::val out_b), {
        ImGui::ColorConvertHSVtoRGB(h, s, v, access_value<float>(out_r), access_value<float>(out_g), access_value<float>(out_b));
    }));

    // Inputs Utilities: Keyboard
    // - For 'int user_key_index' you can use your own indices/enums according to how your backend/engine stored them in io.KeysDown[].
    // - We don't know the meaning of those value. You can use GetKeyIndex() to map a ImGuiKey_ value into the user index.
    // IMGUI_API int           GetKeyIndex(ImGuiKey imgui_key);                                    // map ImGuiKey_* values into user's key index. == io.KeyMap[key]
    // IMGUI_API bool          IsKeyDown(int user_key_index);                                      // is key being held. == io.KeysDown[user_key_index].
    // IMGUI_API bool          IsKeyPressed(int user_key_index, bool repeat = true);               // was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
    // IMGUI_API bool          IsKeyReleased(int user_key_index);                                  // was key released (went from Down to !Down)?
    // IMGUI_API int           GetKeyPressedAmount(int key_index, float repeat_delay, float rate); // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
    // IMGUI_API void          CaptureKeyboardFromApp(bool want_capture_keyboard_value = true);    // attention: misleading name! manually override io.WantCaptureKeyboard flag next frame (said flag is entirely left for your application to handle). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard_value"; after the next NewFrame() call.
    // GetKeyIndex removed in v1.92 - ImGuiKey values are now the indices
    emscripten::function("IsKeyDown", FUNCTION(bool, (int key), { return ImGui::IsKeyDown((ImGuiKey)key); }));
    emscripten::function("IsKeyPressed", FUNCTION(bool, (int key, bool repeat), { return ImGui::IsKeyPressed((ImGuiKey)key, repeat); }));
    emscripten::function("IsKeyReleased", FUNCTION(bool, (int key), { return ImGui::IsKeyReleased((ImGuiKey)key); }));
    emscripten::function("IsKeyChordPressed", FUNCTION(bool, (ImGuiKeyChord key_chord), { return ImGui::IsKeyChordPressed(key_chord); }));
    emscripten::function("GetKeyPressedAmount", FUNCTION(int, (int key, float repeat_delay, float rate), { return ImGui::GetKeyPressedAmount((ImGuiKey)key, repeat_delay, rate); }));
    emscripten::function("GetKeyName", FUNCTION(std::string, (int key), { return std::string(ImGui::GetKeyName((ImGuiKey)key)); }));
    emscripten::function("SetNextFrameWantCaptureKeyboard", &ImGui::SetNextFrameWantCaptureKeyboard);
    emscripten::function("Shortcut", FUNCTION(bool, (ImGuiKeyChord key_chord, ImGuiInputFlags flags), { return ImGui::Shortcut(key_chord, flags); }));
    emscripten::function("SetNextItemShortcut", FUNCTION(void, (ImGuiKeyChord key_chord, ImGuiInputFlags flags), { ImGui::SetNextItemShortcut(key_chord, flags); }));
    emscripten::function("SetItemKeyOwner", FUNCTION(void, (ImGuiKey key), { ImGui::SetItemKeyOwner(key); }));
    // CaptureKeyboardFromApp removed in v1.92 - use SetNextFrameWantCaptureKeyboard instead
    emscripten::function("CaptureKeyboardFromApp", FUNCTION(void, (bool want_capture_keyboard), { ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard); }));

    // Inputs Utilities: Mouse
    // - To refer to a mouse button, you may use named enums in your code e.g. ImGuiMouseButton_Left, ImGuiMouseButton_Right.
    // - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
    // - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
    // IMGUI_API bool          IsMouseDown(ImGuiMouseButton button);                               // is mouse button held?
    // IMGUI_API bool          IsMouseClicked(ImGuiMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down)
    // IMGUI_API bool          IsMouseReleased(ImGuiMouseButton button);                           // did mouse button released? (went from Down to !Down)
    // IMGUI_API bool          IsMouseDoubleClicked(ImGuiMouseButton button);                      // did mouse button double-clicked? (note that a double-click will also report IsMouseClicked() == true)
    // IMGUI_API bool          IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
    // IMGUI_API bool          IsMousePosValid(const ImVec2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    // IMGUI_API bool          IsAnyMouseDown();                                                   // is any mouse button held?
    // IMGUI_API ImVec2        GetMousePos();                                                      // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
    // IMGUI_API ImVec2        GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
    // IMGUI_API bool          IsMouseDragging(ImGuiMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    // IMGUI_API ImVec2        GetMouseDragDelta(ImGuiMouseButton button = 0, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    // IMGUI_API void          ResetMouseDragDelta(ImGuiMouseButton button = 0);                   //
    // IMGUI_API ImGuiMouseCursor GetMouseCursor();                                                // get desired cursor type, reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
    // IMGUI_API void          SetMouseCursor(ImGuiMouseCursor cursor_type);                       // set desired cursor type
    // IMGUI_API void          CaptureMouseFromApp(bool want_capture_mouse_value = true);          // attention: misleading name! manually override io.WantCaptureMouse flag next frame (said flag is entirely left for your application to handle). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse_value;" after the next NewFrame() call.
    emscripten::function("IsMouseDown", &ImGui::IsMouseDown);
    emscripten::function("IsMouseClicked", &ImGui::IsMouseClicked);
    emscripten::function("IsMouseReleased", &ImGui::IsMouseReleased);
    emscripten::function("IsMouseDoubleClicked", &ImGui::IsMouseDoubleClicked);
    emscripten::function("IsMouseHoveringRect", FUNCTION(bool, (emscripten::val r_min, emscripten::val r_max, bool clip), { return ImGui::IsMouseHoveringRect(import_ImVec2(r_min), import_ImVec2(r_max), clip); }));
    emscripten::function("IsMousePosValid", FUNCTION(bool, (emscripten::val mouse_pos), { return ImGui::IsMousePosValid(import_maybe_null_value<ImVec2>(mouse_pos)); }));
    emscripten::function("IsAnyMouseDown", &ImGui::IsAnyMouseDown);
    emscripten::function("GetMousePos", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetMousePos(), out); }));
    emscripten::function("GetMousePosOnOpeningCurrentPopup", FUNCTION(emscripten::val, (emscripten::val out), { return export_ImVec2(ImGui::GetMousePosOnOpeningCurrentPopup(), out); }));
    emscripten::function("IsMouseDragging", &ImGui::IsMouseDragging);
    emscripten::function("GetMouseDragDelta", FUNCTION(emscripten::val, (int button, float lock_threshold, emscripten::val out), { return export_ImVec2(ImGui::GetMouseDragDelta(button, lock_threshold), out); }));
    emscripten::function("ResetMouseDragDelta", &ImGui::ResetMouseDragDelta);
    emscripten::function("GetMouseCursor", &ImGui::GetMouseCursor);
    emscripten::function("SetMouseCursor", &ImGui::SetMouseCursor);
    emscripten::function("SetNextFrameWantCaptureMouse", &ImGui::SetNextFrameWantCaptureMouse);
    // CaptureMouseFromApp removed in v1.92 - use SetNextFrameWantCaptureMouse instead
    emscripten::function("CaptureMouseFromApp", FUNCTION(void, (bool want_capture_mouse), { ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse); }));

    // Clipboard Utilities
    // - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text data to the clipboard.
    // IMGUI_API const char*   GetClipboardText();
    // IMGUI_API void          SetClipboardText(const char* text);
    emscripten::function("GetClipboardText", FUNCTION(std::string, (), { const char* text = ImGui::GetClipboardText(); return (text != NULL) ? text : ""; }));
    emscripten::function("SetClipboardText", FUNCTION(void, (emscripten::val text), { ImGui::SetClipboardText(text.as<std::string>().c_str()); }));

    // Settings/.Ini Utilities
    // - The disk functions are automatically called if io.IniFilename != NULL (default is "imgui.ini").
    // - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description about handling .ini saving manually.
    // IMGUI_API void          LoadIniSettingsFromDisk(const char* ini_filename);                  // call after CreateContext() and before the first call to NewFrame(). NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
    // IMGUI_API void          LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size=0); // call after CreateContext() and before the first call to NewFrame() to provide .ini data from your own data source.
    // IMGUI_API void          SaveIniSettingsToDisk(const char* ini_filename);                    // this is automatically called (if io.IniFilename is not empty) a few seconds after any modification that should be reflected in the .ini file (and also by DestroyContext).
    // IMGUI_API const char*   SaveIniSettingsToMemory(size_t* out_ini_size = NULL);               // return a zero-terminated string with the .ini data which you can save by your own mean. call when io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.
    emscripten::function("LoadIniSettingsFromDisk", FUNCTION(void, (std::string ini_filename), { ImGui::LoadIniSettingsFromDisk(ini_filename.c_str()); }));
    emscripten::function("LoadIniSettingsFromMemory", FUNCTION(void, (std::string ini_data), { ImGui::LoadIniSettingsFromMemory(ini_data.c_str()); }));
    emscripten::function("SaveIniSettingsToDisk", FUNCTION(void, (std::string ini_filename), { ImGui::SaveIniSettingsToDisk(ini_filename.c_str()); }));
    emscripten::function("SaveIniSettingsToMemory", FUNCTION(std::string, (), { return ImGui::SaveIniSettingsToMemory(); }));

    // Debug Utilities
    // IMGUI_API bool          DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx); // This is called by IMGUI_CHECKVERSION() macro.
    emscripten::function("DebugCheckVersionAndDataLayout", FUNCTION(bool, (std::string version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx), {
        return ImGui::DebugCheckVersionAndDataLayout(version_str.c_str(), sz_io, sz_style, sz_vec2, sz_vec4, sz_drawvert, sz_drawidx);
    }));
    emscripten::function("DebugStartItemPicker", &ImGui::DebugStartItemPicker);
    emscripten::function("DebugFlashStyleColor", FUNCTION(void, (int idx), { ImGui::DebugFlashStyleColor((ImGuiCol)idx); }));
    emscripten::function("DebugTextEncoding", FUNCTION(void, (std::string text), { ImGui::DebugTextEncoding(text.c_str()); }));

    // Memory Allocators
    // - All those functions are not reliant on the current context.
    // - If you reload the contents of imgui.cpp at runtime, you may need to call SetCurrentContext() + SetAllocatorFunctions() again because we use global storage for those.
    // IMGUI_API void          SetAllocatorFunctions(void* (*alloc_func)(size_t sz, void* user_data), void (*free_func)(void* ptr, void* user_data), void* user_data = NULL);
    // IMGUI_API void*         MemAlloc(size_t size);
    // IMGUI_API void          MemFree(void* ptr);
    emscripten::function("SetAllocatorFunctions", FUNCTION(void, (emscripten::val alloc_func, emscripten::val free_func, emscripten::val user_data), {
        WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
        ctx->_ImGui_SetAllocatorFunctions_alloc_func = alloc_func;
        ctx->_ImGui_SetAllocatorFunctions_free_func = free_func;
        ctx->_ImGui_SetAllocatorFunctions_user_data = user_data;
        if (alloc_func.isNull() || free_func.isNull()) {
            ImGui::SetAllocatorFunctions(NULL, NULL, NULL);
        } else {
            ImGui::SetAllocatorFunctions(
                FUNCTION(void*, (size_t sz, void* user_data), {
                    WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
                    return ctx->_ImGui_SetAllocatorFunctions_alloc_func(emscripten::val(sz), ctx->_ImGui_SetAllocatorFunctions_user_data).as<void*>(emscripten::allow_raw_pointers());
                }), 
                FUNCTION(void, (void* ptr, void* user_data), {
                    WrapImGuiContext* ctx = WrapImGuiContext::GetCurrentContext();
                    ctx->_ImGui_SetAllocatorFunctions_free_func(emscripten::val(ptr), ctx->_ImGui_SetAllocatorFunctions_user_data);
                }), 
                NULL);
        }
    }));
    emscripten::function("MemAlloc", FUNCTION(emscripten::val, (size_t sz), { void* p = ImGui::MemAlloc(sz); return emscripten::val(p); }), emscripten::allow_raw_pointers());
    emscripten::function("MemFree", FUNCTION(void, (emscripten::val ptr), { void* _ptr = ptr.as<void*>(emscripten::allow_raw_pointers()); ImGui::MemFree(_ptr); }));
    // No equivalent in ImGUI.
    // Allocate memory for a glyph range. Copy over the glyph range sent from javascript.
    // Return a pointer to the glyph range.
    // Can free the pointer using MemFree.
    emscripten::function("GlyphRangeAlloc", FUNCTION(emscripten::val, (emscripten::val glyph_ranges), {
        const size_t ln = glyph_ranges["length"].as<size_t>();
        size_t sz = sizeof(ImWchar) * (ln + 1);
        void* p = ImGui::MemAlloc(sz);
        ImWchar* ch = (ImWchar*)p;
        for (int i = 0; i < ln; i++)
            ch[i] = glyph_ranges[i].as<ImWchar>();
        ch[ln] = 0;
        return emscripten::val((intptr_t) ch);
    }), emscripten::allow_raw_pointers());
    // No equivalent in ImGUI.
    // Given a pointer to a glyph range return the actual glyph range.
    emscripten::function("GlyphRangeExport", FUNCTION(emscripten::val, (emscripten::val glyph_ranges), {
        const ImWchar* p = (ImWchar*) glyph_ranges.as<intptr_t>();
        int length = 0;
        for (const ImWchar* pp = p; *pp != 0; pp++)
            length++;
        return emscripten::val(emscripten::typed_memory_view((size_t)length, p));
    }), emscripten::allow_raw_pointers());
    // (Optional) Platform/OS interface for multi-viewport support
    // Read comments around the ImGuiPlatformIO structure for more details.
    // Note: You may use GetWindowViewport() to get the current viewport of the current window.
    // IMGUI_API ImGuiPlatformIO&  GetPlatformIO();                                                // platform/renderer functions, for backend to setup + viewports list.
    emscripten::function("GetPlatformIO", FUNCTION(emscripten::val, (), {
        ImGuiPlatformIO* p = &ImGui::GetPlatformIO();
        return emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    // IMGUI_API ImGuiViewport*    GetMainViewport();                                              // main viewport. same as GetPlatformIO().MainViewport == GetPlatformIO().Viewports[0].
    emscripten::function("GetMainViewport", FUNCTION(emscripten::val, (), {
        ImGuiViewport* p = ImGui::GetMainViewport();
        return emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    // IMGUI_API void              UpdatePlatformWindows();                                        // call in main loop. will call CreateWindow/ResizeWindow/etc. platform functions for each secondary viewport, and DestroyWindow for each inactive viewport.
    // IMGUI_API void              RenderPlatformWindowsDefault(void* platform_render_arg = NULL, void* renderer_render_arg = NULL); // call in main loop. will call RenderWindow/SwapBuffers platform functions for each secondary viewport which doesn't have the ImGuiViewportFlags_Minimized flag set. May be reimplemented by user for custom rendering needs.
    // IMGUI_API void              DestroyPlatformWindows();                                       // call DestroyWindow platform functions for all viewports. call from backend Shutdown() if you need to close platform windows before imgui shutdown. otherwise will be called by DestroyContext().
    // IMGUI_API ImGuiViewport*    FindViewportByID(ImGuiID id);                                   // this is a helper for backends.
    // IMGUI_API ImGuiViewport*    FindViewportByPlatformHandle(void* platform_handle);            // this is a helper for backends. the type platform_handle is decided by the backend (e.g. HWND, MyWindow*, GLFWwindow* etc.)
    emscripten::function("UpdatePlatformWindows", &ImGui::UpdatePlatformWindows);
    emscripten::function("RenderPlatformWindowsDefault", FUNCTION(void, (emscripten::val platform_render_arg, emscripten::val renderer_render_arg), {
        void* _platform_render_arg = platform_render_arg.isNull() ? NULL : (void*)(intptr_t)platform_render_arg.as<intptr_t>();
        void* _renderer_render_arg = renderer_render_arg.isNull() ? NULL : (void*)(intptr_t)renderer_render_arg.as<intptr_t>();
        ImGui::RenderPlatformWindowsDefault(_platform_render_arg, _renderer_render_arg);
    }));
    emscripten::function("DestroyPlatformWindows", &ImGui::DestroyPlatformWindows);
    emscripten::function("FindViewportByID", FUNCTION(emscripten::val, (ImGuiID id), {
        ImGuiViewport* p = ImGui::FindViewportByID(id);
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());
    emscripten::function("FindViewportByPlatformHandle", FUNCTION(emscripten::val, (emscripten::val platform_handle), {
        ImGuiViewport* p = ImGui::FindViewportByPlatformHandle(platform_handle.isNull() ? NULL : (void*)(intptr_t)platform_handle.as<intptr_t>());
        return p == NULL ? emscripten::val::null() : emscripten::val(p);
    }), emscripten::allow_raw_pointers());
}
