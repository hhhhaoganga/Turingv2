#include "engine.h"
#include "graphics.h"
class component;
enum class pintype{
    input,output
};

class pin{
    component* owner;
    pintype type;
    int index;
    bool state;
public:
    pin(component*,pintype,int);
    void setstate(bool);
    bool getstate() const;
};
pin::pin(component* o,pintype t,int i){
    owner=o;
    type=t;
    index=i;
    state=true;
}
void pin::setstate(bool s){
    state=s;
}
bool pin::getstate() const{
    return state;
}
