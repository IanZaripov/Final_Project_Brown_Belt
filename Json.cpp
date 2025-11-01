#include "json.h"

using namespace std;

namespace Json {

  Document::Document(Node root) : root(move(root)) {
  }

  const Node& Document::GetRoot() const {
    return root;
  }

  Node LoadNode(istream& input);

  Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']'; ) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadDouble(istream& input) {
    if(input.peek() == 't') {
      input.ignore(4);
      return Node(true);
    } 
    else if (input.peek() == 'f') {
      input.ignore(5);
      return Node(false);
    }
    std::string result;
    while(input.peek() == '-' || input.peek() == '.' || isdigit(input.peek())) {
      result += input.get();
    }
    return Node(std::stod(result));
  }

  Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream& input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}'; ) {
      if (c == ',') {
        input >> c;
      }

      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadNode(istream& input) {
    char c;
    input >> c;
    if (c == '[') {
      return LoadArray(input);
    } else if (c == '{') {
      return LoadDict(input);
    } else if (c == '"') {
      return LoadString(input);
    } else {
      input.putback(c);
      return LoadDouble(input);
    }
  }

  Document Load(istream& input) {
    return Document{LoadNode(input)};
  }

}
