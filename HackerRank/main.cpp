#include <iostream>
#include <fstream>

using namespace std;
uint64_t
ntoh64(const uint64_t *input)
{
    uint64_t rval;
    uint8_t *data = (uint8_t *)&rval;

    data[0] = *input >> 56;
    data[1] = *input >> 48;
    data[2] = *input >> 40;
    data[3] = *input >> 32;
    data[4] = *input >> 24;
    data[5] = *input >> 16;
    data[6] = *input >> 8;
    data[7] = *input >> 0;

    return rval;
}

void trimTrailingSpaces(string& s) {
    while (s.size() > 0 && s.back() == ' ') {
        s.pop_back();
    }
}

struct market_update {
    
    int16_t length;
    char type;
    string symbol;
    int16_t size;
    uint64_t price;
    
    
    market_update(char* buffer){
        type = buffer[2];
        if (type == 'T') {
            memcpy(&length, (void*) buffer, sizeof(int16_t));
            length = ntohs(length);
        
            symbol = string(buffer+3, 5);
            trimTrailingSpaces(symbol);
            memcpy(&size, (void *) (buffer+8), sizeof(int16_t));
            size = ntohs(size);
            
            memcpy(&price, (void *) (buffer+10), sizeof(uint64_t));
            price = ntoh64(&price);
            
        }
    }
};

void parse_market_updates (market_update* update, ofstream& out) {
    if (update->type == 'T'){
//        printf("%d %s @ %.2f\n", update->size*100, update->symbol.c_str(), update->price/10000.);
        char output[1024];
        sprintf(output, "%d %s @ %.2f\n", update->size*100, update->symbol.c_str(), update->price/10000.);
        out << string(output) << endl;
    }
}

void print_market_updates (market_update* update) {
    cout << "update information" << endl
         << "type: " << update->type << endl
         << "symbol: " << update->symbol << endl
         << "size: " << update->size << endl
         << "price: " << update->price << endl;
}

void parse_packet (char* start_ptr, ofstream& output) {
    int16_t packet_length;
    int16_t updates_number;
    int offset = 0;
    memcpy(&packet_length, (void *)start_ptr, sizeof(int16_t));
    packet_length = ntohs(packet_length);
    offset += 2;
    memcpy(&updates_number, (void *)(start_ptr+offset), sizeof(int16_t));

    updates_number = ntohs(updates_number);
    offset += 2;
    
    for (int i = 0; i < updates_number; ++i) { //Loop through all the numbers of update in the packet
        market_update* update = new market_update(start_ptr+offset);
        parse_market_updates(update, output);
        offset += update->length;
        delete update;
    }
    
}
int main() {
    // insert code here...
    long length;
    char *buffer;
    ifstream fin("input.dat", ios:: binary);
    if ( !fin){
        cout << "Fail opening input file" << endl;
        return 0;
    }
    ofstream fout("output.txt");
    if( !fout) {
        cout << "Fail opening an output file" << endl;
        return 0;
    }
    //Get the size of the file
    fin.seekg(0, ios::end);
    length = fin.tellg();
    fin.seekg(0, ios::beg);
    
    buffer = new char[length*2];
    fin.read(buffer,length);

    int16_t size = 0;
    int offset = 0;
    
  
    while (offset < length) {
       
        memcpy(&size, (void*)(buffer + offset), sizeof(int16_t));
        size = ntohs(size);
        parse_packet(buffer+offset, fout);
        offset += size;
        
    }

    fin.close();
    fout.close();
    
    return 0;
}
