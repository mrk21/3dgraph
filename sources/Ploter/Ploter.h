/**
 * @file   Ploter.h
 * @author 村田 裕一
 * @brief  グラフの要素とそれらを統括するクラス郡を定義したファイル。
 *///---------------------------------------------------------------------------
#ifndef  INCLUDED_PLOTER_H
#define  INCLUDED_PLOTER_H

#include  <iostream>
#include  <fstream>
#include  <sstream>
#include  <list>
#include  <deque>
#include  <boost/shared_ptr.hpp>
#include  <boost/mem_fn.hpp>
#include  <boost/utility.hpp>
#include  "Utility.h"

namespace Ploter {
    
    class HiddenLineManager;
    
    /**
     * @brief 観察対象を表すクラス。
     *///---------------------------------------------------------------------------
    class Subject {
        
      public:
        
        virtual       std::size_t  getIntervalNumber (void) const { return 0;                        }
        virtual const VertexList & getPositions      (void) const { return Subject::dummyVertexList; }
        virtual const Vector     & getDirection      (void) const { return Subject::dummyVector;     }
        virtual       bool         isSettablePosition(int ) const { return false;                    }
        
      private:
        
        static VertexList  dummyVertexList;
        static Vector      dummyVector;
    };
    
    VertexList  Subject::dummyVertexList;
    Vector      Subject::dummyVector;
    
    
    /**
     * @brief 捜査対象を表すクラス。
     *///---------------------------------------------------------------------------
    class Scannee {
        
      public:
        
        virtual void  acceptScanning     ( HiddenLineManager * )       = 0;
        virtual void  acceptComplementing( HiddenLineManager * )       = 0;
        virtual void  draw               ( void                ) const = 0;
    };
    
    
    /**
     * @brief 。
     *///---------------------------------------------------------------------------
    class ScanLine {
        
      public:
        
        ScanLine( Element scanLine ) : position(scanLine) {
            
            minmax.initialize();
        }
        
        friend bool operator <  ( const ScanLine & leftOP,  const ScanLine & rightOP ) { return    leftOP.position <  rightOP.position;   }
        friend bool operator <= ( const ScanLine & leftOP,  const ScanLine & rightOP ) { return    leftOP.position <= rightOP.position;   }
        friend bool operator == ( const ScanLine & leftOP,  const ScanLine & rightOP ) { return    leftOP.position == rightOP.position;   }
        friend bool operator >  ( const ScanLine & leftOP,  const ScanLine & rightOP ) { return !( leftOP          <  rightOP          ); }
        friend bool operator >= ( const ScanLine & leftOP,  const ScanLine & rightOP ) { return !( leftOP          <= rightOP          ); }
        friend bool operator != ( const ScanLine & leftOP,  const ScanLine & rightOP ) { return !( leftOP          == rightOP          ); }
        
        Element  position;
        MinMax   minmax;
    };
    
    
    /**
     * @brief 陰線処理を管理するクラス。
     *///---------------------------------------------------------------------------
    class HiddenLineManager {
        
      public:
        
        typedef  std::list<ScanLine>            ScanLineList;
        typedef  std::list<ScanLine>::iterator  ScanLineListIterator;
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        HiddenLineManager( Scannee * target ) {
            
            if ( Option("Graph").get("isHiddenLine") ) {
                
                this->resetPosition();
                
                target->acceptScanning(this);
                
                this->scanLines.sort  ();
                this->scanLines.unique();
                
                target->acceptComplementing(this);
            }
            
            target->draw();
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  resetPosition(void) {
            
            this->currentScanLine = this->scanLines.begin();
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  addScanLine( Element scanLine ) {
            
            this->scanLines.push_back(scanLine);
        }
        
        ScanLineListIterator  currentScanLine;
        
      private:
        
        ScanLineList  scanLines;
    };
    
    
    /**
     * @brief 複合グラフ要素を表すクラス。
     *///---------------------------------------------------------------------------
    template<class ChildType>
    class Composite {
        
      public:
        
        /**
         * @brief childを子要素に設定する。
         *///---------------------------------------------------------------------------
        void  setChild( ChildType * child ) {
            
            this->child = shared_ptr<ChildType>(child);
        }
        
        /**
         * @brief 子要素を取得する。
         *///---------------------------------------------------------------------------
        const ChildType * getChild(void) const {
            
            return this->child.get();
        }
        
        /**
         * @brief 自身と子要素を行列affineで座標変換した後、描画する。
         *///---------------------------------------------------------------------------
        void  draw( const Matrix & affine ) const {
            
            this->drawMyself(affine);
            
            if ( this->child != 0 ) { this->child->draw(affine); }
        }
        
      private:
        
        virtual void  drawMyself( const Matrix & affine ) const = 0;
        
      protected:
        
        shared_ptr<ChildType>  child;
    };
    
    
    /**
     * @brief グラフ要素の集合を表すクラス。
     *///---------------------------------------------------------------------------
    template<class ChildType>
    class Collection : public Scannee {
        
      public:
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  acceptScanning( HiddenLineManager * hiddenLineManager ) {
            
            this->accept( hiddenLineManager, &Scannee::acceptScanning  );
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  acceptComplementing( HiddenLineManager * hiddenLineManager ) {
            
            this->accept( hiddenLineManager, &Scannee::acceptComplementing );
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        virtual void  draw(void) const {
            
            std::for_each( this->children.begin(), this->children.end(), mem_fn( &Scannee::draw ) );
        }
        
      protected:
        
        std::deque<ChildType>  children;
        
      private:
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  accept( HiddenLineManager * hiddenLineManager,  void (Scannee::*accept)(HiddenLineManager *) ) {
            
            std::for_each( this->children.begin(), this->children.end(),
                
                bind( accept, _1, hiddenLineManager )
            );
        }
    };
    
    
    /**
     * @brief 線分を表すクラス。
     *///---------------------------------------------------------------------------
    class Line : public Scannee {
        
      public:
        
        /**
         * @brief 始点begin_と終点end_を結ぶ線分を生成する。
         *///---------------------------------------------------------------------------
        Line( const Vector & begin_, const Vector & end_) :
            
            begin( begin_), end( end_), source(X), target(Y)
        {}
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        const Vector & getBegin(void) const { return this->begin; }
        const Vector & getEnd  (void) const { return this->begin; }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  draw(void) const {
            
            if ( Option("Graph").get("isHiddenLine") ) {
                
                for ( std::list<Vector>::const_iterator  it = this->points.begin();  next(it) != this->points.end();  it++ ) {
                    
                    drawVertex( *      it  );
                    drawVertex( * next(it) );
                }
                
            } else {
                
                drawVertex( this->begin );
                drawVertex( this->end   );
            }
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  acceptScanning( HiddenLineManager * hiddenLineManager ) {
            
            this->findCharacter();
            
            hiddenLineManager->addScanLine( this->begin(this->source) );
            hiddenLineManager->addScanLine( this->end  (this->source) );
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  acceptComplementing( HiddenLineManager * hiddenLineManager ) {
            
            this->points.clear();
            
            while ( this->isInRange( hiddenLineManager->currentScanLine->position ) ) {
                
                Vector  point(V::W_UNIT);
                
                point(this->source) = hiddenLineManager->currentScanLine->position;
                point(this->target) = this->f( point(this->source) );
                point(Z           ) = this->begin(this->target);
                
                if ( hiddenLineManager->currentScanLine->minmax.assess( point(this->target) ) != MinMax::INNER ) {
                    
                    this->points.push_back(point);
                }
                
                hiddenLineManager->currentScanLine++;
            }
            
            hiddenLineManager->currentScanLine--;
        }
        
      private:
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  pushBackPoint( HiddenLineManager * hiddenLineManager,  const Vector & point ) {
            
            if ( (hiddenLineManager->currentScanLine++)->minmax.assess( point(this->target) ) ) {
                
                this->points.push_back(point);
            }
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        bool  isInRange( Element scanLine ) {
            
            return  ( this->begin(this->source) <= scanLine ) && ( scanLine <= this->end(this->source) );
        }
        
        /**
         * @brief 線分の特性を求める。
         *///---------------------------------------------------------------------------
        void  findCharacter(void) {
            
            Element  x1 = this->begin(this->source),  x2 = this->end(this->source);
            Element  y1 = this->begin(this->target),  y2 = this->end(this->target);
            
            this->delta     = ( y2 - y1 ) / ( x2 - x1 );
            this->intercept =   y1 - this->delta * x1;
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        Element  f( Element x ) const {
            
            return  this->delta*x + this->intercept;
        }
        
        const Vector &     begin;
        const Vector &     end;
        Element            delta;
        Element            intercept;
        std::size_t        source;
        std::size_t        target;
        std::list<Vector>  points;
    };
    
    
    /**
     * @brief 色を表すクラス。
     *///---------------------------------------------------------------------------
    class RGBA {
        
      public:
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        RGBA( double r_ = 0.0, double g_ = 0.0, double b_ = 0.0, double a_ = 1.0 ) :
            
            r( r_), g( g_), b( b_), a( a_)
        {}
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  set(void) const {
            
            glColor4d( this->r, this->g, this->b, this->a );
        }
        
        double  r;
        double  g;
        double  b;
        double  a;
    };
    
    
    /**
     * @brief ポリラインを表すクラス。
     *///---------------------------------------------------------------------------
    class PolyLine : public Collection<Line> {
        
      public:
        
        /**
         * @brief [begin,end)の範囲の頂点の集合からポリラインを生成する。
         *///---------------------------------------------------------------------------
        template<class IteratorType>
        PolyLine( IteratorType begin,  IteratorType end ) {
            
            for ( IteratorType it = begin;  it != end-1;  it++ ) {
                
                this->children.push_back( Line( *it, *(it+1) ) );
            }
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  acceptComplementing( HiddenLineManager * hiddenLineManager ) {
            
            hiddenLineManager->resetPosition();
            
            for ( Element firstScanLine = this->children.begin()->getBegin()(X);
                
                hiddenLineManager->currentScanLine->position < firstScanLine;
                hiddenLineManager->currentScanLine++
            );
            
            this->Collection<Line>::acceptComplementing( hiddenLineManager );
        }
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        void  draw(void) const {
            
            if ( Option("Graph").get("isHiddenLine") ) { this->color = RGBA(0,0,0,0.2); }
            else                                       { this->color = RGBA(0,0,0,0.7); }
            
            this->color.set();
            
            glBegin( GL_LINES ); {
                
                this->Collection<Line>::draw();
                
            } glEnd();
        }
        
        mutable RGBA  color;
    };
    
    
    /**
     * @brief ポリラインの集合を表すクラス。
     *///---------------------------------------------------------------------------
    class PolyLineList : public Collection<PolyLine> {
        
      public:
        
        /**
         * @brief [begin,end)の範囲の頂点の集合からポリラインのリストを生成する。
         *///---------------------------------------------------------------------------
        template<class IteratorType>
        void  set( IteratorType begin,  IteratorType end ) {
            
            this->children.clear();
            
            for ( IteratorType it = begin;  it != end;  it++ ) {
                
                this->children.push_back( PolyLine( it.begin(), it.end() ) );
            }
        }
    };
    
    
    /**
     * @brief 目盛を表すクラス。
     *///---------------------------------------------------------------------------
    class Tics : public Composite<Tics>, public Subject {
        
      public:
        
        /**
         * @brief 目盛線がline_、親要素がparent_、オプションがoption_の目盛を生成する。
         *///---------------------------------------------------------------------------
        Tics( const Option & option_, const Vector & line_, const Subject * parent_) :
            
            option( option_), line( line_), parent( parent_)
        {}
        
        /**
         * @brief 位置リストを取得する。
         *///---------------------------------------------------------------------------
        const VertexList & getPositions(void) const {
            
            return this->positions;
        }
        
        /**
         * @brief 目盛の方向を取得する。
         *///---------------------------------------------------------------------------
        const Vector & getDirection(void) const {
            
            return this->parent->getDirection();
        }
        
        /**
         * @brief 区間数を取得する。
         *///---------------------------------------------------------------------------
        std::size_t  getIntervalNumber(void) const {
            
            return  this->option.get("divisionNumber") * this->parent->getIntervalNumber();
        }
        
        /**
         * @brief iが設定可能な場所かどうかを取得する。 
         *///---------------------------------------------------------------------------
        bool  isSettablePosition(int i) const {
            
            return  i % this->child->option.get("divisionNumber");
        }
        
      private:
        
        /**
         * @brief 目盛線を行列affineで座標変換した後、描画する。 
         *///---------------------------------------------------------------------------
        void  drawMyself( const Matrix & affine ) const {
            
            if ( this->option.get("isDraw") ) {
                
                this->setPosition();
                
                drawParallelLines( affine, this->positions, this->line );
            }
        }
        
        /**
         * @brief 目盛線の位置の設定。
         *///---------------------------------------------------------------------------
        void  setPosition(void) const {
            
            Vector  distance( this->parent->getDirection() / this->getIntervalNumber() );
            
            this->positions.clear();
            
            for ( Index i=0;  i <= this->getIntervalNumber();  i++ ) {
                
                if ( this->parent->isSettablePosition(i) ) {
                    
                    this->positions.push_back( i * distance );
                }
            }
        }
        
        const   Option      option;
        const   Subject *   parent;
        const   Vector      line;
        mutable VertexList  positions;
    };
    
    
    /**
     * @brief 座標軸を表すクラス。
     *///---------------------------------------------------------------------------
    class Axis : public Composite<Tics>, public Subject {
        
      public:
        
        /**
         * @brief 方向がdirection_の座標軸を生成する。
         *///---------------------------------------------------------------------------
        Axis( const Option & option_, const Vector & direction_) :
            
            option( option_), direction( direction_)
        {}
        
        /**
         * @brief 座標軸の方向を取得する。
         *///---------------------------------------------------------------------------
        const Vector & getDirection(void) const {
            
            return this->direction;
        }
        
        /**
         * @brief 区間数を取得する。
         *///---------------------------------------------------------------------------
        std::size_t  getIntervalNumber(void) const {
            
            return 1;
        }
        
        /**
         * @brief iが設定可能な場所かどうかを取得する。 
         *///---------------------------------------------------------------------------
        bool  isSettablePosition(int i) const {
            
            return true;
        }
        
      private:
        
        /**
         * @brief 座標軸を行列affineで座標変換した後、描画する。
         *///---------------------------------------------------------------------------
        void  drawMyself( const Matrix & affine ) const {
            
            if ( this->option.get("isDraw") ) {
                
                glBegin( GL_LINES ); {
                    
                    drawLine( affine, V::ZERO, this->direction );
                    
                } glEnd();
            }
        }
        
        const Option  option;
        const Vector  direction;
    };
    
    
    /**
     * @brief グリッドを表すクラス。
     *///---------------------------------------------------------------------------
    class Grid : public Composite<Grid> {
        
      public:
        
        /**
         * @brief オプションがoption_で目盛tics1_と目盛tics2_についてのグリッドを生成する。
         *///---------------------------------------------------------------------------
        Grid( const Option & option_, const Subject * tics1_, const Subject * tics2_) :
            
            option( option_), tics1( tics1_), tics2( tics2_)
        {}
        
      private:
        
        /**
         * @brief 格子線を行列affineで座標変換した後、描画する。
         *///---------------------------------------------------------------------------
        void  drawMyself( const Matrix & affine ) const {
            
            if ( this->option.get("isDraw") ) {
                
                glEnable( GL_LINE_STIPPLE );  {
                    
                    glLineStipple( 1, 0x8888 );
                    
                    drawParallelLines( affine, this->tics1->getPositions(), this->tics2->getDirection() );
                    drawParallelLines( affine, this->tics2->getPositions(), this->tics1->getDirection() );
                    
                } glDisable( GL_LINE_STIPPLE );
            }
        }
        
        const Option    option;
        const Subject * tics1;
        const Subject * tics2;
    };
    
    
    /**
     * @brief ３次元グラフを表すクラス。
     *///---------------------------------------------------------------------------
    class Graph {
        
      public:
        
        typedef std::vector<std::string>       Data;
        typedef std::istringstream             DataStream;
        typedef std::istream_iterator<Element> DataStreamIterator;
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        Graph( const Option & option_) : option( option_) {}
        
        /**
         * @brief 最小値・最大値を取得する。
         *///---------------------------------------------------------------------------
        const MinMaxManager & getMinMaxManager(void) const {
            
            return  this->minmaxManager;
        }
        
        /**
         * @brief fileNameが指すファイルから、頂点を設定する。
         *///---------------------------------------------------------------------------
        void  set( const char * path ) {
            
            Data  data;
            
            Graph::scanData(path,data);
            
            this->setSize  (data);
            this->setVertex(data);
            this->setPolyLine();
        }
        
        /**
         * @brief 描画する。
         *///---------------------------------------------------------------------------
        void  draw( const AffineManager * affineManager,  int target ) const {
            
            this->operateVertex();
            this->minmaxManager.set( transformed );
            
            this->draw( affineManager->get(target) );
        }
        
      private:
        
        /**
         * @brief データの読み込み。
         *///---------------------------------------------------------------------------
        static void  scanData( const char * path,  Data & data ) {
            
            std::ifstream  ifs(path);
            std::string    buffer;
            
            while ( std::getline( ifs, buffer ) ) {
                
                data.push_back( buffer );
            }
        }
        
        /**
         * @brief 頂点数の設定。
         *///---------------------------------------------------------------------------
        void  setSize( const Data & data ) {
            
            std::size_t  xNumber = 0;
            std::size_t  yNumber = data.size();
            
            DataStream          ds(data[0]);
            DataStreamIterator  current(ds);
            DataStreamIterator  end;
            
            while ( current++ != end ) { ++xNumber; }
            
            this->   original.resize( yNumber, xNumber );
            this->transformed.resize( yNumber, xNumber );
        }
        
        /**
         * @brief 頂点の設定。
         *///---------------------------------------------------------------------------
        void  setVertex( const Data & data ) {
            
            for ( Index j=0;  j < this->original.size1();  j++ ) {
                
                DataStream          ds(data[j]);
                DataStreamIterator  k(ds);
                
                for ( Index i=0;  i < this->original.size2();  i++ ) {
                    
                    this->original(j,i) = v( i,j, *(k++) );
                }
            }
        }
        
        /**
         * @brief ポリラインの設定。
         *///---------------------------------------------------------------------------
        void  setPolyLine(void) {
            
            this->xPolyLines.set( this->transformed. begin1(), this->transformed. end1() );
            this->yPolyLines.set( this->transformed.rbegin2(), this->transformed.rend2() );
        }
        
        /**
         * @brief 頂点の操作。 
         *///---------------------------------------------------------------------------
        void  operateVertex(void) const {
            
            this->transformed = this->original;
            
            if ( this->option.get("isRegularizeX") ) { regularize( this->transformed.begin1(), this->transformed.end1() ); }
            if ( this->option.get("isRegularizeY") ) { regularize( this->transformed.begin2(), this->transformed.end2() ); }
            if ( this->option.get("isLogScale"   ) ) { toLogScale( this->transformed,          V::Z                     ); }
        }
        
        /**
         * @brief グラフを行列affineで座標変換した後、描画する。
         *///---------------------------------------------------------------------------
        void  draw( const Matrix & affine ) const {
            
            dualForEach( this->transformed.begin1(),
                         this->transformed.end1  (),  bind( transform, affine, _1 ) );
            
            if ( this->option.get("isDrawXPolyLines") ) { HiddenLineManager( &this->xPolyLines ); }
            if ( this->option.get("isDrawYPolyLines") ) { HiddenLineManager( &this->yPolyLines ); }
        }
        
        const   Option         option;
                VertexMatrix   original;
        mutable VertexMatrix   transformed;
        mutable PolyLineList   xPolyLines;
        mutable PolyLineList   yPolyLines;
        mutable MinMaxManager  minmaxManager;
    };
    
    
    /**
     * @brief グラフ要素を統括するクラス。 
     *///---------------------------------------------------------------------------
    class Ploter {
        
      public:
        
        /**
         * @brief 。
         *///---------------------------------------------------------------------------
        Ploter(void) : affineManager( Option("affine"), &this->graph.getMinMaxManager() ),
                               graph( Option("graph" )                                  ) {
            
            this->xAxis = makeAxis( "x",        V::X_UNIT, -V::Z_UNIT );
            this->yAxis = makeAxis( "y",        V::Y_UNIT, -V::Z_UNIT );
            this->zAxis = makeAxis( "z",  0.5 * V::Z_UNIT, -V::X_UNIT );
            
            this->xyGrid = makeGrid( "xy", this->xAxis, this->yAxis );
            this->xzGrid = makeGrid( "xz", this->xAxis, this->zAxis );
            this->yzGrid = makeGrid( "yz", this->yAxis, this->zAxis );
        }
        
        /**
         * @brief 設定する。
         *///---------------------------------------------------------------------------
        void  set( const char * fileName ) {
            
            graph.set( fileName );
        }
        
        /**
         * @brief 描画する。
         *///---------------------------------------------------------------------------
        void  draw(void) const  {
            
            glClearColor( 1.0, 1.0, 1.0, 1.0  );
            glClear     ( GL_COLOR_BUFFER_BIT );
            
            glLoadIdentity();
            glOrtho( -0.4, 2.0,  -0.4, 2.0,  -10.0, 10.0 );
            
            glEnable( GL_BLEND ); {
                
                glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA );
                
                this->drawAxis ();
                this->drawGrid ();
                this->drawGraph();
                
            } glDisable( GL_BLEND );
        }
        
        /**
         * @brief ウィンドウサイズ変化時のビューポートの再設定。
         *///---------------------------------------------------------------------------
        void  resize( int width,  int height ) const {
            
            int  max   = std::max(width,height);
            int  min   = std::min(width,height);
            int  space = (max - min) / 2;
            
            if ( min == height ) { glViewport( space, 0, min, min ); }
            else                 { glViewport( 0, space, min, min ); }
            
            this->draw();
        }
        
      private:
        
        /**
         * @brief 軸の向きがaxisDirection、目盛線の向きがticsDirection、識別名がnameの
         *        座標軸を生成する。
         *///---------------------------------------------------------------------------
        static shared_ptr<Axis>  makeAxis( const std::string & name,  const Vector & axisDirection,
                                                                      const Vector & ticsDirection ) {
            
            Option   axisOption( name +  "Axis" );
            Option   ticsOption( name +  "Tics" );
            Option  mticsOption( name + "MTics" );
            
            Axis *  axis = new Axis(  axisOption,         axisDirection        );
            Tics *  tics = new Tics(  ticsOption,  0.04 * ticsDirection,  axis );
            Tics * mtics = new Tics( mticsOption,  0.02 * ticsDirection,  tics );
            
            axis->setChild( tics);
            tics->setChild(mtics);
            
            return shared_ptr<Axis>(axis);
        }
        
        /**
         * @brief 軸axis1と軸axis2を張る識別名nameのグリッド平面を生成する。
         *///---------------------------------------------------------------------------
        static shared_ptr<Grid>  makeGrid( const std::string & name,  const shared_ptr<Axis> axis1,
                                                                      const shared_ptr<Axis> axis2 ) {
            
            Option   gridOption( name +  "Grid" );
            Option  mgridOption( name + "MGrid" );
            
            Grid * grid = new Grid(  gridOption,  axis1->getChild(),
                                                   axis2->getChild() );
            Grid * mgrid = new Grid( mgridOption,  axis1->getChild()->getChild(),
                                                   axis2->getChild()->getChild() );
            
            grid->setChild(mgrid);
            
            return shared_ptr<Grid>(grid);
        }
        
        /**
         * @brief 座標軸の描画。
         *///---------------------------------------------------------------------------
        void  drawAxis(void) const {
            
            glColor4d( 0.0,  0.0,  0.0,  1.0 );
            
            xAxis->draw( affineManager.get( AffineManager::FOR_X_AXIS ) );
            yAxis->draw( affineManager.get( AffineManager::FOR_Y_AXIS ) );
            zAxis->draw( affineManager.get( AffineManager::FOR_Z_AXIS ) );
        }
        
        /**
         * @brief グリッドの描画。
         *///---------------------------------------------------------------------------
        void  drawGrid(void) const {
            
            glColor4d( 0.0,  0.0,  0.0,  0.7 );
            
            this->xyGrid->draw( this->affineManager.get( AffineManager::FOR_XY_GRID ) );
            this->xzGrid->draw( this->affineManager.get( AffineManager::FOR_XZ_GRID ) );
            this->yzGrid->draw( this->affineManager.get( AffineManager::FOR_YZ_GRID ) );
        }
        
        /**
         * @brief グラフの描画。
         *///---------------------------------------------------------------------------
        void  drawGraph(void) const {
            
            glEnable( GL_LINE_SMOOTH );  {
                
                glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
                
                graph.draw( &this->affineManager, AffineManager::FOR_GRAPH );
                
            }  glDisable( GL_LINE_SMOOTH );
        }
        
        shared_ptr<Axis>  xAxis;
        shared_ptr<Axis>  yAxis;
        shared_ptr<Axis>  zAxis;
        shared_ptr<Grid>  xyGrid;
        shared_ptr<Grid>  xzGrid;
        shared_ptr<Grid>  yzGrid;
        Graph             graph;
        AffineManager     affineManager;
    };
}
#endif
