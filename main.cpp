#include <iostream>
#include <string>
#include <list>
#include <map>
#include <functional>

using namespace std;


class Entity {
private:
    int x, y;
    int vida;

public:
    Entity() : x(0), y(0), vida(100) {}

    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    void heal(int n) {
        vida += n;
    }

    void damage(int n) {
        vida -= n;
    }

    void reset() {
        x = 0;
        y = 0;
        vida = 100;
    }

    string status() const {
        return "Pos(" + to_string(x) + "," + to_string(y) + ") Vida: " + to_string(vida);
    }
};


using Command = function<void(const list<string>&)>;


class CommandCenter {
private:
    map<string, Command> comandos;
    list<string> historial;
    map<string, list<pair<string, list<string>>>> macros;
    Entity& entity;

public:
    CommandCenter(Entity& e) : entity(e) {}

    void registroComando(const string& name, Command cmd) {
        comandos[name] = cmd;
    }

    void ElminacionComando(const string& name) {
        auto it = comandos.find(name);
        if (it != comandos.end()) {
            comandos.erase(it);
            cout << "Comando eliminado: " << name << endl;
        } else {
            cout << "Comando no existe\n";
        }
    }

    void ejecutar(const string& name, const list<string>& args) {
        auto it = comandos.find(name);

        if (it == comandos.end()) {
            cout << "Comando no encontrado: " << name << endl;
            return;
        }

        string antes = entity.status();

        it->second(args);

        string despues = entity.status();

        historial.push_back(name + " | " + antes + " -> " + despues);
    }

    void mostrarHistorial() {
        cout << "\n=== HISTORIAL ===\n";
        for (auto it = historial.begin(); it != historial.end(); ++it) {
            cout << *it << endl;
        }
    }

    // ===== MACROS =====
    void registerMacro(const string& name,
        const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }

    void executeMacro(const string& name) {
        auto it = macros.find(name);

        if (it == macros.end()) {
            cout << "Macro no encontrado\n";
            return;
        }

        for (auto step = it->second.begin(); step != it->second.end(); ++step) {
            if (comandos.find(step->first) == comandos.end()) {
                cout << "Error: comando " << step->first << " no existe\n";
                return;
            }
            ejecutar(step->first, step->second);
        }
    }
};


void healCommand(Entity& e, const list<string>& args) {
    if (args.size() != 1) {
        cout << "Error: heal necesita 1 argumento\n";
        return;
    }

    int val = stoi(*args.begin());
    e.heal(val);
}


class DamageCommand {
private:
    Entity& entity;
    int contador;

public:
    DamageCommand(Entity& e) : entity(e), contador(0) {}

    void operator()(const list<string>& args) {
        if (args.size() != 1) {
            cout << "Error: damage necesita 1 argumento\n";
            return;
        }

        int val = stoi(*args.begin());
        entity.damage(val);
        contador++;

        cout << "Damage usado " << contador << " veces\n";
    }
};


int main() {

    Entity entity;
    CommandCenter center(entity);




    center.registroComando("move", [&](const list<string>& args) {
        if (args.size() != 2) {
            cout << "Error: move necesita 2 argumentos\n";
            return;
        }

        auto it = args.begin();
        int x = stoi(*it++);
        int y = stoi(*it);

        entity.move(x, y);
    });


    center.registroComando("heal", [&](const list<string>& args) {
        healCommand(entity, args);
    });


    center.registroComando("damage", DamageCommand(entity));

    center.registroComando("status", [&](const list<string>& args) {
        cout << entity.status() << endl;
    });


    center.registroComando("reset", [&](const list<string>& args) {
        entity.reset();
    });

    center.ejecutar("heal", {"10"});
    center.ejecutar("move", {"5", "3"});
    center.ejecutar("damage", {"4"});
    center.ejecutar("status", {});

    list<pair<string, list<string>>> macro1 = {
        {"heal", {"20"}},
        {"move", {"2", "2"}},
        {"status", {}}
    };

    list<pair<string, list<string>>> macro2 = {
        {"damage", {"5"}},
        {"status", {}}
    };

    list<pair<string, list<string>>> macro3 = {
        {"reset", {}},
        {"status", {}}
    };

    center.registerMacro("boost", macro1);
    center.registerMacro("attack", macro2);
    center.registerMacro("restart", macro3);

    center.executeMacro("boost");
    center.executeMacro("attack");
    center.executeMacro("restart");

    center.ElminacionComando("heal");
    center.ejecutar("heal", {"10"}); // debe fallar

    center.mostrarHistorial();

    cout << "\nEstado final: " << entity.status() << endl;

    return 0;
}