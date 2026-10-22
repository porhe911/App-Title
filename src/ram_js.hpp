// SPDX-License-Identifier: GPL-3.0-or-later
// Bounded edits of existing React element calls. No system module is embedded.
#pragma once
#include <cstddef>
#include <cstdio>
#include <cstring>
namespace app_title::ram_js {
struct Span {std::size_t a=0,b=0;};
inline bool letter(char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'||c=='$';}
inline bool digit(char c){return c>='0'&&c<='9';}
inline bool white(char c){return c==' '||c=='\t'||c=='\r'||c=='\n';}
struct Token {Span s{};unsigned kind=0;}; // 1=name, 2=number, 3=string/regexp, 4=punctuation
struct Lexer {
    const char* s;std::size_t n,p;bool expression=true,ok=true;
    Lexer(const char* text,std::size_t size,std::size_t at=0):s(text),n(size),p(at){}
    Token next() {
        while(p<n) {
            if(white(s[p])){++p;continue;}
            if(p+1<n&&s[p]=='/'&&s[p+1]=='/'){p+=2;while(p<n&&s[p]!='\n')++p;continue;}
            if(p+1<n&&s[p]=='/'&&s[p+1]=='*') {
                p+=2;while(p+1<n&&(s[p]!='*'||s[p+1]!='/'))++p;
                if(p+1>=n){ok=false;return {}; }p+=2;continue;
            }
            break;
        }
        if(p>=n)return {};
        const auto begin=p;const char c=s[p++];unsigned kind=4;
        if(letter(c)) {
            kind=1;while(p<n&&(letter(s[p])||digit(s[p])))++p;
            expression=(p-begin==6&&!std::memcmp(s+begin,"return",6))||
                (p-begin==5&&!std::memcmp(s+begin,"throw",5))||
                (p-begin==4&&!std::memcmp(s+begin,"case",4));
        } else if(digit(c)) {kind=2;while(p<n&&(letter(s[p])||digit(s[p])||s[p]=='.'))++p;expression=false;}
        else if(c=='\''||c=='\"'||c=='`'||(c=='/'&&expression)) {
            kind=3;bool closed=false,character_class=false;
            while(p<n) {
                const char ch=s[p++];
                if(ch=='\\'){if(p<n)++p;else break;continue;}
                if(c=='/'&&ch=='[')character_class=true;
                if(c=='/'&&ch==']')character_class=false;
                if(ch==c&&!character_class){closed=true;break;}
                if(c=='/'&&(ch=='\n'||ch=='\r'))break;
            }
            if(!closed){ok=false;return {};}
            if(c=='/')while(p<n&&letter(s[p]))++p;
            expression=false;
        } else {
            if(p<n&&((c=='&'&&s[p]=='&')||(c=='|'&&s[p]=='|')||(c=='='&&s[p]=='=')))++p;
            expression=c!=')'&&c!=']'&&c!='}';
        }
        return {{begin,p},kind};
    }
};
struct Document {
    const char* s;std::size_t n;
    explicit Document(const char* text):s(text),n(std::strlen(text)){}
    Span trim(Span r) const {while(r.a<r.b&&white(s[r.a]))++r.a;while(r.b>r.a&&white(s[r.b-1]))--r.b;return r;}
    bool equal(Span r,const char* text) const {r=trim(r);return r.b-r.a==std::strlen(text)&&!std::memcmp(s+r.a,text,r.b-r.a);}
    bool string(Span r,const char* text) const {r=trim(r);return r.b-r.a==std::strlen(text)+2&&(s[r.a]=='\''||s[r.a]=='\"')&&s[r.b-1]==s[r.a]&&!std::memcmp(s+r.a+1,text,r.b-r.a-2);}
    bool same(Span a,Span b) const {a=trim(a);b=trim(b);return a.b-a.a==b.b-b.a&&!std::memcmp(s+a.a,s+b.a,a.b-a.a);}
    bool identifier(Span r) const {r=trim(r);if(r.a==r.b||!letter(s[r.a]))return false;for(auto p=r.a+1;p<r.b;++p)if(!letter(s[p])&&!digit(s[p]))return false;return true;}
    bool code_at(std::size_t p) const {
        Lexer l(s,n);for(Token t=l.next();t.kind;t=l.next())if(t.s.b>p)return l.ok&&t.s.a==p&&t.kind!=3;
        return false;
    }
    bool list(std::size_t open,char left,char right,Span* parts,unsigned cap,unsigned& count,std::size_t& end) const {
        if(open>=n||s[open]!=left)return false;
        Lexer l(s,n,open+1);char stack[64];unsigned depth=0;std::size_t begin=open+1;count=0;
        for(Token t=l.next();t.kind;t=l.next()) {
            if(t.kind!=4||t.s.b-t.s.a!=1)continue;
            const char c=s[t.s.a];
            if(c==right&&!depth) {
                const auto part=trim({begin,t.s.a});
                if(part.a!=part.b){if(count==cap)return false;parts[count++]=part;}
                end=t.s.b;return l.ok;
            }
            if(c==','&&!depth){if(count==cap)return false;parts[count++]=trim({begin,t.s.a});begin=t.s.b;continue;}
            if(c=='('||c=='['||c=='{'){if(depth==sizeof(stack))return false;stack[depth++]=c;}
            else if(c==')'||c==']'||c=='}') {
                if(!depth)return false;
                const char expected=c==')'?'(':c==']'?'[':'{';if(stack[--depth]!=expected)return false;
            }
        }
        return false;
    }
    struct Call {Span whole{},factory{};Span args[8]{};unsigned count=0;};
    bool call(std::size_t at,Call& out) const {
        constexpr char tail[]=".default.createElement(";
        const char* found=std::strstr(s+at,tail);if(!found)return false;
        const auto dot=static_cast<std::size_t>(found-s);std::size_t begin=dot;
        if(begin>=2&&s[begin-1]==')'&&s[begin-2]=='(')begin-=2;
        const auto name_end=begin;while(begin&& (letter(s[begin-1])||digit(s[begin-1])))--begin;
        if(!identifier({begin,name_end})||!code_at(begin))return false;
        const auto open=dot+sizeof(tail)-2;std::size_t end=0;unsigned count=0;
        if(!list(open,'(',')',out.args,8,count,end))return false;
        out.whole={begin,end};out.factory={begin,open};out.count=count;return true;
    }
    bool property(Span object,const char* key,Span& value) const {
        object=trim(object);Span fields[32];unsigned count=0;std::size_t end=0;
        if(!list(object.a,'{','}',fields,32,count,end)||end!=object.b)return false;
        unsigned matches=0;
        for(unsigned i=0;i<count;++i) {
            Lexer l(s,fields[i].b,fields[i].a);const auto name=l.next(),colon=l.next();
            if((equal(name.s,key)||string(name.s,key))&&equal(colon.s,":")){value=trim({colon.s.b,fields[i].b});++matches;}
        }
        return matches==1;
    }
    bool wrapper(unsigned id,Span& require_name) const {
        Lexer l(s,n);const char* tokens[]={"__haul_application",".","l","("};
        for(const auto* t:tokens)if(!equal(l.next().s,t))return false;
        char number[16];std::snprintf(number,sizeof(number),"%u",id);
        if(!equal(l.next().s,number)||!equal(l.next().s,","))return false;
        auto t=l.next();if(equal(t.s,"("))t=l.next();
        if(!equal(t.s,"function")||!equal(l.next().s,"("))return false;
        for(unsigned i=0;i<3;++i){t=l.next();if(!identifier(t.s))return false;if(i==2)require_name=t.s;else if(!equal(l.next().s,","))return false;}
        return equal(l.next().s,")")&&equal(l.next().s,"{");
    }
    bool imported(Span alias,Span require_name,unsigned id) const {
        Lexer l(s,n);unsigned matches=0;char number[16];std::snprintf(number,sizeof(number),"%u",id);
        for(Token t=l.next();t.kind;t=l.next())if(t.kind==1&&same(t.s,alias)) {
            auto look=l;if(!equal(look.next().s,"="))continue;
            auto fn=look.next();if(!identifier(fn.s)||!equal(look.next().s,"("))continue;
            if(!same(fn.s,require_name)){fn=look.next();if(!same(fn.s,require_name)||!equal(look.next().s,"("))continue;}
            if(equal(look.next().s,number)&&equal(look.next().s,")"))++matches;
        }
        return matches==1;
    }
    bool style_alias(Span value,const char* field,Span& alias) const {
        Lexer l(s,value.b,value.a);alias=l.next().s;
        return identifier(alias)&&equal(l.next().s,".")&&equal(l.next().s,"default")&&
            equal(l.next().s,".")&&equal(l.next().s,field)&&!l.next().kind;
    }
};
struct Builder {
    char text[4096]{};std::size_t used=0,limit;bool ok=true;
    explicit Builder(std::size_t capacity):limit(capacity<sizeof(text)?capacity:sizeof(text)){}
    void add(const char* p,std::size_t n){if(!ok||n>=limit-used){ok=false;return;}std::memcpy(text+used,p,n);used+=n;text[used]=0;}
    void add(const char* p){add(p,std::strlen(p));}
    void add(const Document& d,Span r){if(r.b<r.a||r.b>d.n){ok=false;return;}add(d.s+r.a,r.b-r.a);}
};
inline bool native_type(const Document& d,Span value,const char* suffix,Span& prefix) {
    value=d.trim(value);const auto length=std::strlen(suffix);
    if(value.b-value.a<=length||std::memcmp(d.s+value.b-length,suffix,length))return false;
    prefix={value.a,value.b-length};return true;
}
inline bool props(Builder& b,const Document& d,Span object,Span* style_override=nullptr) {
    Span fields[32];unsigned count=0;std::size_t end=0;
    if(!d.list(object.a,'{','}',fields,32,count,end)||end!=object.b)return false;
    b.add("{");unsigned emitted=0;
    for(unsigned i=0;i<count;++i) {
        Lexer l(d.s,fields[i].b,fields[i].a);const auto name=l.next(),colon=l.next();
        if(d.equal(name.s,"__source")&&d.equal(colon.s,":")) {
            const auto meta=d.trim({colon.s.b,fields[i].b});Span file,line;
            if(d.property(meta,"fileName",file)&&d.property(meta,"lineNumber",line)) {
                Lexer f(d.s,file.b,file.a),ln(d.s,line.b,line.a);const auto ft=f.next(),nt=ln.next();
                if((ft.kind==1||ft.kind==3)&&!f.next().kind&&nt.kind==2&&!ln.next().kind)continue;
            }
        }
        if(emitted++)b.add(",");
        if(style_override&&d.equal(name.s,"style")&&d.equal(colon.s,":")) {
            b.add("style:[");b.add(d,*style_override);b.add(",{marginTop:48}]");
        } else b.add(d,fields[i]);
    }
    b.add("}");return b.ok;
}
inline bool outside_tile(char* source,std::size_t capacity) {
    Document d(source);Span require_name;if(!d.wrapper(552,require_name))return false;
    Document::Call image,mat,label;Span native,style,alias,gate,outer_style{};unsigned matches=0,arrays=0;
    for(std::size_t from=0;from<d.n;) {
        const auto* found=std::strstr(source+from,".default.createElement(");if(!found)break;
        from=static_cast<std::size_t>(found-source)+1;Document::Call c;Span type;
        if(!d.call(from-1,c)||c.count!=3||!native_type(d,c.args[0],".Image",type))continue;
        if(!d.property(c.args[1],"style",style)||!d.style_alias(style,"image",alias)||!d.imported(alias,require_name,219))return false;
        image=c;native=type;++matches;
    }
    if(matches!=1||!d.call(image.args[2].a,mat)||mat.whole.a!=image.args[2].a||mat.whole.b!=image.args[2].b||mat.count!=3)return false;
    Span mat_native;if(!native_type(d,mat.args[0],".Animated.View",mat_native)||!d.same(native,mat_native))return false;
    if(!d.call(mat.args[2].a,label)||label.whole.b!=mat.args[2].b||label.count!=3)return false;
    gate=d.trim({mat.args[2].a,label.whole.a});Lexer g(source,gate.b,gate.a);Token last{};
    for(Token t=g.next();t.kind;t=g.next())last=t;
    if(!g.ok||!d.equal(last.s,"&&"))return false;
    Span label_native,label_style,label_alias;
    if(!native_type(d,label.args[0],".Text",label_native)||!d.same(native,label_native)||
       !d.property(label.args[1],"style",label_style)||!d.style_alias(label_style,"titleId",label_alias)||!d.same(alias,label_alias))return false;
    Lexer scan(source,d.n);
    for(Token t=scan.next();t.kind;t=scan.next())if(d.equal(t.s,"[")) {
        Span items[4];unsigned count=0;std::size_t end=0;
        if(!d.list(t.s.a,'[',']',items,4,count,end)||count!=2)continue;
        Span first,second;if(d.style_alias(items[0],"imageWrapper",first)&&d.style_alias(items[1],"image",second)&&d.same(first,alias)&&d.same(second,alias)) {
            outer_style={t.s.a,end};++arrays;
        }
    }
    if(arrays!=1||outer_style.a<image.whole.b)return false;
    Builder b(capacity);b.add(d,{0,image.whole.a});b.add(d,image.factory);b.add("(");b.add(d,native);b.add(".View,{style:");b.add(d,alias);b.add(".default.imageWrapper},");
    b.add(d,image.factory);b.add("(");b.add(d,image.args[0]);b.add(",");if(!props(b,d,image.args[1],&style))return false;b.add(",");
    b.add(d,mat.factory);b.add("(");b.add(d,mat.args[0]);b.add(",");if(!props(b,d,mat.args[1]))return false;b.add(")),");
    b.add(d,gate);b.add(d,label.factory);b.add("(");b.add(d,label.args[0]);b.add(",");if(!props(b,d,label.args[1]))return false;b.add(",");b.add(d,label.args[2]);b.add("))");
    b.add(d,{image.whole.b,outer_style.a});b.add("[");b.add(d,alias);b.add(".default.imageWrapper]");b.add(d,{outer_style.b,d.n});
    if(!b.ok)return false;
    std::memcpy(source,b.text,b.used+1);std::memset(source+b.used,' ',capacity-b.used-1);source[capacity-1]=0;return true;
}
inline bool strand_priority(char* source,std::size_t capacity) {
    Document d(source);Span require_name;if(!d.wrapper(527,require_name))return false;
    Span chosen{},items[4];unsigned matches=0;bool reversed=false;
    for(std::size_t from=0;from<d.n;) {
        const auto* found=std::strstr(source+from,".default.createElement(");if(!found)break;
        from=static_cast<std::size_t>(found-source)+1;Document::Call c;Span id,style,type;
        if(!d.call(from-1,c)||c.count<2||!native_type(d,c.args[0],".Animated.View",type)||!d.property(c.args[1],"testID",id)||!d.string(id,"strand-container")||!d.property(c.args[1],"style",style))continue;
        unsigned count=0;std::size_t end=0;if(!d.list(style.a,'[',']',items,4,count,end)||end!=style.b||count!=2)return false;
        Span alias;unsigned base=0;if(!d.style_alias(items[0],"strandContainer",alias)){base=1;if(!d.style_alias(items[1],"strandContainer",alias))return false;}
        if(!d.imported(alias,require_name,25))return false;
        const auto startup=items[1-base];constexpr char suffix[]=".strandContainer";
        if(startup.b-startup.a<=sizeof(suffix)-1||std::memcmp(source+startup.b-(sizeof(suffix)-1),suffix,sizeof(suffix)-1)||
           !d.identifier({startup.a,startup.b-(sizeof(suffix)-1)}))return false;
        chosen=style;reversed=base==1;++matches;
    }
    if(matches!=1)return false;
    if(reversed)return true;
    Span values[4];unsigned count=0;std::size_t end=0;if(!d.list(chosen.a,'[',']',values,4,count,end))return false;
    char replacement[512];const auto n=std::snprintf(replacement,sizeof(replacement),"[%.*s,%.*s]",static_cast<int>(values[1].b-values[1].a),source+values[1].a,static_cast<int>(values[0].b-values[0].a),source+values[0].a);
    if(n<0||static_cast<std::size_t>(n)>=sizeof(replacement))return false;
    const auto old=chosen.b-chosen.a;if(d.n-old+static_cast<std::size_t>(n)>=capacity)return false;
    std::memmove(source+chosen.a+n,source+chosen.b,d.n-chosen.b+1);std::memcpy(source+chosen.a,replacement,static_cast<std::size_t>(n));
    const auto used=d.n-old+static_cast<std::size_t>(n);std::memset(source+used,' ',capacity-used-1);source[capacity-1]=0;return true;
}
}
