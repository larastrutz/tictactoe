#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <mutex>
#include <chrono>

// Classe TicTacToe
class TicTacToe {
  private:
  std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
  char current_player; // Jogador atual ('X' ou 'O')
  bool game_over; // Estado do jogo
  char winner; // Vencedor do jogo
  
  std::mutex mtx; 
  
  public:
  TicTacToe() {
    // Inicializar o tabuleiro e as variáveis do jogo
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        board[i][j] = ' ';
      }
    }
    winner = '-';
    game_over = false;
    // sorteia jogador inicial entre 'X' e 'O'
    static std::mt19937 sorteiaJogador(static_cast<unsigned int>(time(0)));
    static std::uniform_int_distribution<int> distr(0, 1);
    current_player = distr(sorteiaJogador) == 0 ? 'X' : 'O';
  }
  
  void display_board() {
    // Exibir o tabuleiro no console
    std::system("clear");
    for(int i = 0; i < 3; i++){
      std::cout<<board[i][0] << "|" << board[i][1]<< "|" << board[i][2] << std::endl;
      if(i != 2){
        std::cout << "-----" << std::endl;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  
  bool make_move(char player, int row, int col) {
    // Alternância de turnos com Espera Ocupada
    // Fica em loop até que seja a vez do jogador atual
    while (true) {
        mtx.lock();

        if (game_over) {
            mtx.unlock();
            return true; // Se o jogo acabou, solta o lock e manda a thread parar
        }

        if (current_player == player) {
            // É a vez deste jogador
            // Quebra o loop mas mantém o mutex travado para a seção crítica
            break; 
        }

        // Se não for a vez dele, solta o lock para o outro jogador poder acessar
        mtx.unlock(); 
        
        // Dá uma pausa para não usar 100% da CPU na espera ocupada
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Seção Crítica
    bool jogada_valida = false;

    if(board[row][col] == ' ') {
        board[row][col] = player;
        display_board();
        
        // Verifica se esta jogada encerrou o jogo
        if (check_win(player)) {
            winner = player;
            game_over = true;
        } else if (check_draw()) {
            winner = 'D';
            game_over = true;
        }

        // Alterna o jogador
        current_player = (player == 'X') ? 'O' : 'X';
        
        jogada_valida = true;
    } 

    // Protocolo de saída
    mtx.unlock();
    
    // Retorna false se a posição estava ocupada (para o jogador tentar outra casa)
    // Retorna true se a jogada foi feita com sucesso
    return jogada_valida; 
  }
  
  bool check_win(char player) {
    // linhas
    for(int i = 0; i < 3; i++){
      if(player == board[i][0] && player == board[i][1] && player == board[i][2]){
        return true;
      }
    }
    // colunas
    for(int i = 0; i < 3; i++){
      if(player == board[0][i] && player == board[1][i] && player == board[2][i]){
        return true;
      }
    }
    // diagonal
    if(player == board[0][0] && player == board[1][1] && player == board[2][2]){
      return true;
    }
    if(player == board[0][2] && player == board[1][1] && player == board[2][0]){
      return true;
    }
    return false;
  }
  
  bool check_draw() {
    // Verificar se houve um empate
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(board[i][j] == ' '){
          return false;
        }
      }
    }
    return true;
  }
  
  char get_winner() {
    // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
    // Lock para não ler o vencedor enquanto a outra thread escreve
    std::lock_guard<std::mutex> lock(mtx);
    return winner;
  }
};

// Classe Player
class Player {
  private:
  TicTacToe& game; // Referência para a instância do jogo
  char symbol; // Símbolo do jogador ('X' ou 'O')
  std::string strategy; // Estratégia do jogador
  
  public:
  Player(TicTacToe& g, char s, std::string strat) 
  : game(g), symbol(s), strategy(strat) {}
  
  void play() {
    // Executar jogadas de acordo com a estratégia escolhida
    while(game.get_winner() == '-'){
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      if(strategy == "sequential"){
        play_sequential();
      }else{
        play_random();
      }
    }
  }
  
  private:
  void play_sequential() {
    // Implementar a estratégia sequencial de jogadas
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(game.make_move(symbol, i, j)){
          return;
        }
      }
    }
  }
  
  void play_random() {
    // Implementar a estratégia aleatória de jogadas
    int l;
    int c;
    bool fim = false;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distr(0, 2);
    while(!fim){
      l = distr(gen);
      c = distr(gen);
      fim = game.make_move(symbol, l, c);
    } 
  }
};

// Função principal
int main() {
  // Inicializar o jogo e os jogadores
  TicTacToe tabuleiro;
  tabuleiro.display_board();
  Player X(tabuleiro, 'X', "sequential");
  Player O(tabuleiro, 'O', "random");
  
  // Criar as threads para os jogadores
  std::thread Jogador1(&Player::play, &X);
  std::thread Jogador2(&Player::play, &O);
  
  // Aguardar o término das threads
  Jogador1.join();
  Jogador2.join();
  
  // Exibir o resultado final do jogo
  char vencedor = tabuleiro.get_winner();
  if(vencedor == 'D'){
    std::cout<<" Empate!\n";
  }else{
    std::cout<<" Vencedor: "<<vencedor<<"\n";
  }
  
  return 0;
}